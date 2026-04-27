#ifndef METRICS_H
#define METRICS_H

#include "transaction.h"

void record_transaction_result(Transaction* tx);
void print_metrics_report();

#endif