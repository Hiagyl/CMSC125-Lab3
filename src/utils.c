#include "transaction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

OpType get_op_type(char* op_str) {
    if (strcmp(op_str, "DEPOSIT") == 0) return OP_DEPOSIT;
    if (strcmp(op_str, "WITHDRAW") == 0) return OP_WITHDRAW;
    if (strcmp(op_str, "TRANSFER") == 0) return OP_TRANSFER;
    if (strcmp(op_str, "BALANCE") == 0) return OP_BALANCE;
    return OP_BALANCE; 
}

int parse_trace(const char* filename, Transaction* workload) {
    FILE* fp = fopen(filename, "r");
    if (!fp) return 0;

    char line[256];
    int count = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;

        char tx_label[10], op_label[20];
        int start_tick, acct_id, amount = 0, target = -1;

        // format: T1 1 BALANCE 10
        int fields = sscanf(line, "%s %d %s %d", tx_label, &start_tick, op_label, &acct_id);
        if (fields < 4) continue;

        Transaction* tx = &workload[count];
        tx->tx_id = atoi(&tx_label[1]);
        tx->start_tick = start_tick;
        tx->num_ops = 1;
        tx->wait_ticks = 0;
        tx->status = TX_RUNNING;

        Operation* op = &tx->ops[0];
        op->type = get_op_type(op_label);
        op->account_id = acct_id;

        if (op->type == OP_TRANSFER) {
            sscanf(line, "%*s %*d %*s %*d %d %d", &target, &amount);
            op->target_account = target;
            op->amount_centavos = amount;
        } else if (op->type != OP_BALANCE) {
            sscanf(line, "%*s %*d %*s %*d %d", &amount);
            op->amount_centavos = amount;
        }
        count++;
    }
    fclose(fp);
    return count;
}