#include "transaction.h"
#include "bank.h"
#include "timer.h"
#include "lock_mgr.h"
#include "buffer_pool.h"
#include <stdio.h>

extern BufferPool pool;

const char* op_to_str(OpType type) {
    switch(type) {
        case OP_DEPOSIT: return "DEPOSIT";
        case OP_WITHDRAW: return "WITHDRAW";
        case OP_TRANSFER: return "TRANSFER";
        case OP_BALANCE: return "BALANCE";
        default: return "UNKNOWN";
    }
}

void* execute_transaction(void* arg) {
    Transaction* tx = (Transaction*)arg;

    // 1. Wait until the simulation reaches the scheduled start time
    wait_until_tick(tx->start_tick);
    tx->actual_start = global_tick;
    tx->status = TX_RUNNING;

    for (int i = 0; i < tx->num_ops; i++) {
        Operation* op = &tx->ops[i];

        // Load into Buffer
        if (op->type == OP_TRANSFER) {
            int first = (op->account_id < op->target_account) ? op->account_id : op->target_account;
            int second = (op->account_id < op->target_account) ? op->target_account : op->account_id;
            load_account(&pool, first);
            load_account(&pool, second);
        } else {
            load_account(&pool, op->account_id);
        }
        
        // force sleep to hold slots longer so we can see if T6 takes it after the first 5 threads
        // usleep(50000);

        // Log Start
        if (op->type == OP_TRANSFER) {
            printf("  T%d started: TRANSFER from %d to %d amount PHP %d.%02d\n", 
                   tx->tx_id, op->account_id, op->target_account, op->amount_centavos/100, op->amount_centavos%100);
        } else {
            printf("  T%d started: %s account %d amount PHP %d.%02d\n", 
                   tx->tx_id, op_to_str(op->type), op->account_id, op->amount_centavos/100, op->amount_centavos%100);
        }

        int tick_before = global_tick;

        switch (op->type) {
            case OP_DEPOSIT:
                deposit(op->account_id, op->amount_centavos);
                printf("  T%d completed: DEPOSIT successful\n", tx->tx_id);
                break;

            case OP_WITHDRAW:
                if (withdraw(op->account_id, op->amount_centavos)) {
                    printf("  T%d completed: WITHDRAW successful\n", tx->tx_id);
                } else {
                    printf("  T%d: WITHDRAW failed (Insufficient Funds)\n", tx->tx_id);
                    tx->status = TX_ABORTED;
                    unload_account(&pool, op->account_id); // unload before exiting
                    return NULL;
                }
                break;

            case OP_TRANSFER:
                // Calls the deadlock-safe transfer in lock_mgr.c
                if (transfer(tx->tx_id, op->account_id, op->target_account, op->amount_centavos)) {
                    printf("  T%d completed: TRANSFER successful\n", tx->tx_id);
                } else {
                    tx->status = TX_ABORTED;
                    // unload both before exiting
                    unload_account(&pool, op->account_id);
                    unload_account(&pool, op->target_account);
                    return NULL;
                }
                break;

            case OP_BALANCE: {
                int balance = get_balance(op->account_id);
                printf("T%d: Account %d balance PHP %d.%02d\n", 
                        tx->tx_id, op->account_id, balance / 100, balance % 100);
                break;
            }
        }
        // Track how many ticks were spent waiting for locks
        tx->wait_ticks += (global_tick - tick_before);

        // Unload from Buffer
        if (op->type == OP_TRANSFER) {
            unload_account(&pool, op->account_id);
            unload_account(&pool, op->target_account);
        } else {
            unload_account(&pool, op->account_id);
        }
    }

    tx->actual_end = global_tick;
    tx->status = TX_COMMITTED;
    return NULL;
}