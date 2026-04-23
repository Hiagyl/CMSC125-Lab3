#include "transaction.h"
#include "bank.h"
#include "timer.h"
#include "lock_mgr.h"
#include <stdio.h>

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
                    return NULL;
                }
                break;

            case OP_TRANSFER:
                // Calls the deadlock-safe transfer in lock_mgr.c
                if (transfer(tx->tx_id, op->account_id, op->target_account, op->amount_centavos)) {
                    printf("  T%d completed: TRANSFER successful\n", tx->tx_id);
                } else {
                    tx->status = TX_ABORTED;
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
    }

    tx->actual_end = global_tick;
    tx->status = TX_COMMITTED;
    return NULL;
}