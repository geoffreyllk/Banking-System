#include <stdio.h>
#include <stdlib.h> 
#include <stdint.h>
#include <stdbool.h> 
#include <string.h> 
#include <ctype.h> 
#include <time.h> 
#include <errno.h>
#include <limits.h>

// Bank account structure
struct Account {
    char name[100];
    int accountNumber;
    int type; // 0 = savings, 1 = current
    char pin[5];
    float balance;
};

struct Account acc;

// log transactions
void logTransaction(const char *message) {
    FILE *transactionLog;

    transactionLog = fopen("database/transaction.log", "a");
    if (transactionLog == NULL) return;
    time_t t = time(NULL);
    fprintf(transactionLog, "[%s] %s\n", ctime(&t), message);
    fclose(transactionLog);
}

// --- 1. create functions ---

// check if an account number is already taken
int isAccountNumberUnique(int accountNumber) {
    FILE *indexFile; 
    indexFile = fopen("database/index.txt", "r");
    // check if file is empty
    if (indexFile == NULL) {
        return 1;
    }

    int existingNumber;
    while (fscanf(indexFile, "%d", &existingNumber) == 1) {
        if (existingNumber == accountNumber) {
            fclose(indexFile);
            return 0;
        }
    }

    fclose(indexFile);
    return 1;
}

void saveAccountNumber(int accountNumber) {
    FILE *indexFile;
    indexFile = fopen("database/index.txt", "a");
    if (indexFile == NULL) {
        printf("Error: couldn’t open index file.\n");
        return;
    }
    fprintf(indexFile, "%d\n", accountNumber);
    fclose(indexFile);
}

void createAccount() {
    printf("Please fill in the following: \n");
    printf("Full Name: ");
    scanf(" %[^\n]", acc.name);

    int running = 1;
    // validate 0 or 1 for account type
    while (running) {
        printf("Account type (0 = savings, 1 = current): ");
        if (scanf("%d", &acc.type) != 1) { // check if input is a number
            printf("Invalid input. Please enter 0 or 1.\n");
            while (getchar() != '\n'); // clear input buffer
            continue;
        }
        if (acc.type == 0 || acc.type == 1) {
            break;
        } else {
            printf("Invalid type. Please enter 0 or 1 only.\n");
        }
    }

    // verify password via double entry
    int pin1, pin2;
    while (running) {
        printf("Set 4-digit PIN: ");
        if (scanf("%d", &pin1) != 1) {
            printf("Invalid input. Please enter digits only.\n");
            while (getchar() != '\n');
            continue;
        }

        if (pin1 < 1000 || pin1 > 9999) {
            printf("Invalid PIN. Must be 4 digits.\n");
            continue;
        }

        printf("Re-enter PIN to confirm: ");
        if (scanf("%d", &pin2) != 1) {
            printf("Invalid input. Please enter digits only.\n");
            while (getchar() != '\n');
            continue;
        }

        if (pin1 == pin2) {
            sprintf(acc.pin, "%04d", pin1);
            break;
        } else {
            printf("PINs do not match. Please try again.\n");
        }
    }

    // account balance (default 0)
    acc.balance = 0.0;

    // generate random account number
    srand(time(NULL));
    int accountNumber;
    do {
        accountNumber = rand() % (999999999 - 1000000 + 1) + 1000000;
    } while (!isAccountNumberUnique(accountNumber));

    acc.accountNumber = accountNumber;
    saveAccountNumber(accountNumber);

    // create filename based on account number
    char filename[100];
    sprintf(filename, "database/%d.txt", acc.accountNumber);

    FILE *accFile = fopen(filename, "w");
    if (accFile == NULL) {
        printf("Error: could not create account file.\n");
        return;
    }

    // write account info to file
    fprintf(accFile, "Name: %s\n", acc.name);
    fprintf(accFile, "Account Number: %d\n", acc.accountNumber);
    if (acc.type == 0) {
        fprintf(accFile, "Account Type: Savings\n");
    } else {
        fprintf(accFile, "Account Type: Current\n");
    }
    fprintf(accFile, "PIN: %s\n", acc.pin);
    fprintf(accFile, "Balance: %.2f\n", acc.balance);

    fclose(accFile);

    printf("\nAccount created successfully!\n");
}




int main() {
    time_t t = time(NULL);
    printf("--- Banking Sytem ---\n");
    printf("Session start: %s\n", ctime(&t));
    printf("Loaded Accounts: %d\n\n", 0);

    char choice[20];

    int running = 1;
    while (running) {
        printf("----------------------------------");
        printf("Please choose the following (1-6): ");
        printf("\n1. Create Account\n");
        printf("2. Delete Account\n");
        printf("3. Deposit\n");
        printf("4. Withdraw\n");
        printf("5. Remittance\n");
        printf("6. Exit\n");
        printf("Select option: ");
        scanf("%s", choice);

        if (strcmp(choice, "1") == 0 || strcmp(choice, "create") == 0) {
            printf("Creating account...\n");
            createAccount();
            logTransaction("create account");
        } 
        else if (strcmp(choice, "2") == 0 || strcmp(choice, "delete") == 0) {
            printf("Deleting account...\n");
            logTransaction("delete account");
        } 
        else if (strcmp(choice, "3") == 0 || strcmp(choice, "deposit") == 0) {
            printf("Depositing...\n");
            logTransaction("deposit");
        } 
        else if (strcmp(choice, "4") == 0 || strcmp(choice, "withdraw") == 0) {
            printf("Withdrawing...\n");
            logTransaction("withdrawal");
        } else if (strcmp(choice, "5") == 0 || strcmp(choice, "remittance") == 0) {
            printf("Remitting funds...");
            logTransaction("remittance");
        } else if (strcmp(choice, "6") == 0 || strcmp(choice, "exit") == 0) {
            printf("Thank you, goodbye. Exiting...!\n");
            logTransaction("session ended");
            break;
        } 
        else {
            printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}