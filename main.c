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
    char ID[13];
    int accountNumber;
    char type[10];
    char pin[5];
    float balance;
};

struct Account acc;

int countAccounts() {
    FILE *indexFile = fopen("database/index.txt", "r");
    if (!indexFile) return 0;

    int count = 0, number;
    while (fscanf(indexFile, "%d", &number) == 1) {
        count++;
    }

    fclose(indexFile);
    return count;
}

// log transactions
void logTransaction(const char *message) {
    FILE *transactionLog;

    transactionLog = fopen("database/transaction.log", "a");
    if (transactionLog == NULL) return;
    time_t t = time(NULL);

    char *timeStr = ctime(&t);
    timeStr[strcspn(timeStr, "\n")] = 0; // remove newline

    fprintf(transactionLog, "[%s] %s\n", timeStr, message);
    fclose(transactionLog);
}

// check if account number exists in index.txt
int isAccountNumberInIndex(int accNum) {
    FILE *indexFile = fopen("database/index.txt", "r");
    if (!indexFile) {
        printf("Error: couldn’t open index file.\n");
        return 0;
    }

    int number;
    int found = 0;
    while (fscanf(indexFile, "%d", &number) == 1) {
        if (number == accNum) {
            found = 1;
            break;
        }
    }

    fclose(indexFile);
    return found;
}

// verifyAccount function for delete(requireID), deposit, withdraw and remittance(account to be transferred, no ID or PIN required)
int verifyAccount(int requireID, int *returnAccountNumber) {
    int running = 1;
    while (running) {
        int accNumInput;
        char pinInput[5];
        char idInput[5];

        int accountFound = 1;
        // verify account number
        while (accountFound) {
            printf("Enter your account number: ");
            scanf("%d", &accNumInput);

            if (!isAccountNumberInIndex(accNumInput)) {
                printf("Account number not found. Please try again.\n");
            } else {
                accountFound = 0;
            }
        }

        // get data of account number inputted from file to compare
        char filename[128];
        sprintf(filename, "database/%d.txt", accNumInput);

        FILE *accFile;
        accFile = fopen(filename, "r");
        if (!accFile) {
            printf("Account not found.\n");
            return 0;
        }

        char storedName[100], storedPIN[5], storedID[13], typeStr[20];
        int storedAccNum;
        float balance;

        fscanf(accFile, "Name: %[^\n]\n", storedName);
        fscanf(accFile, "Account Number: %d\n", &storedAccNum);
        fscanf(accFile, "Account Type: %[^\n]\n", typeStr);
        fscanf(accFile, "PIN: %4s\n", storedPIN);
        fscanf(accFile, "Balance: %f\n", &balance);
        fscanf(accFile, "ID: %12s\n", storedID);

        fclose(accFile);

        if (requireID) {
            int idFound = 1;
            char last4IDs[5];

            // copy last 4 characters of stored ID
            int len = strlen(storedID);
            strncpy(last4IDs, storedID + len - 4, 4);
            last4IDs[4] = '\0';
            // verify id (compare idInput with last 4 char of ID)
            while (idFound) {
                printf("Enter last 4 characters of your ID: ");
                scanf(" %4s", idInput);

                // compare strings
                if (strcmp(idInput, last4IDs) != 0) {
                    printf("Incorrect ID. Try again.\n");
                } else {
                    idFound = 0;
                }
            }
        }

        // verify pin with 4 attempts
        int attemptsLeft = 4;
        while (attemptsLeft > 0) {
            printf("Enter your 4-digit PIN: ");
            scanf(" %4s", pinInput);

            // compare pin inputted with stored pin
            if (strcmp(pinInput, storedPIN) == 0) {
                *returnAccountNumber = accNumInput; // if pins are same (correct), return account number for use
                return 1;
            } else {
                attemptsLeft--; // if pins are not same (wrong), decrement attempt and exit if no more attempts left
                if (attemptsLeft == 0) {
                    printf("You have run out of attempts. Returning to main menu.\n");
                    return 0;
                } else {
                    printf("Incorrect PIN. You have: %d attempts left.", attemptsLeft);
                }
            }
        }
    }
    return 0;
}

// --- 1. create account functions ---

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
    scanf(" %99[^\n]", acc.name); // limit name to max 99 chars

    int valid = 0;
    // check if ID inputted is less than 13 char & is integers
    while (!valid) {
        printf("Identification Number (ID): ");
        scanf(" %12[^\n]", acc.ID);

        valid = 1;
        for (int i = 0; i < strlen(acc.ID); i++) {
            if (!isdigit(acc.ID[i])) {
                valid = 0;
                printf("Invalid ID. Only numbers allowed.\n");
                break;
            }
        }
        if (valid && (strlen(acc.ID) < 8 || strlen(acc.ID) > 12)) {
            valid = 0;
            printf("Invalid ID length. Must be 8-12 digits.\n");
        }
    }

    int running = 1;
    // validate 0 or 1 for account type
    while (running) {
        char typeInput[10];
        printf("Account type (0 = savings, 1 = current): ");
        scanf("%9s", typeInput);

        if (strcmp(typeInput, "0") == 0) {
            strcpy(acc.type, "Savings");
            break;
        } else if (strcmp(typeInput, "1") == 0) {
            strcpy(acc.type, "Current");
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
    } while (isAccountNumberInIndex(accountNumber)); // keep generating a random number until it is not in index.txt

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
    fprintf(accFile, "ID: %s\n", acc.ID);
    fprintf(accFile, "Account Number: %d\n", acc.accountNumber);
    fprintf(accFile, "Account Type: %s\n", acc.type);
    fprintf(accFile, "PIN: %s\n", acc.pin);
    fprintf(accFile, "Balance: %.2f\n", acc.balance);

    fclose(accFile);

    printf("\nAccount created successfully!\n");
}


// --- 2. delete functions ---
void getAccounts() {
    FILE *indexFile;
    indexFile = fopen("database/index.txt", "r");
    if (indexFile == NULL) {
        printf("Error: couldn’t open index file to retrieve account numbers.\n");
        return;
    }
    char line[128];
    printf("Saved Accounts:\n");
    while (fgets(line, sizeof(line), indexFile) != NULL) {
        printf("- %s", line);
    }
    fclose(indexFile);
}

void deleteAccount() {
    // print all account numbers in database
    getAccounts();

    int accountNumber;

    // confirm deletion
    if (verifyAccount(1, &accountNumber)) {
        int running = 1;
        while (running) {
            char confirm;
            printf("Are you sure you want to delete your account? (y/n): ");
            scanf(" %c", &confirm); // only 1 character allowed + skip any blank whitespace
            confirm = tolower(confirm); // convert to lowercase

            if (confirm == 'y') {
                // create filename string of account number e.g. 'database/1234567.txt'
                char filename[128];
                sprintf(filename, "database/%d.txt", accountNumber);

                FILE *accFile;
                accFile = fopen(filename, "r");
                // check if account file exists, if not return not found
                if (!accFile) {
                    printf("Account not found.\n");
                    return;
                }
                fclose(accFile);

                
                if (remove(filename) == 0) {
                    // since cannot directly delete files in c, read all account numbers NOT to be deleted, and write them to a different temp file
                    FILE *indexRead = fopen("database/index.txt", "r");
                    // error check
                    if (!indexRead) {
                        printf("Error: Could not open index.txt for reading.\n");
                        return;
                    }

                    FILE *indexTemp = fopen("database/temp_index.txt", "w");
                    if (!indexTemp) {
                        printf("Error: Could not create temporary index file.\n");
                        fclose(indexRead);  // close indexFile too because its not null (from prev error check)
                        return;
                    }

                    int number;
                    while (fscanf(indexRead, "%d", &number) == 1) {
                        // if NOT the account number to be deleted
                        if (number != accountNumber) {
                            // write to temporary file
                            fprintf(indexTemp, "%d\n", number);
                        }
                    }

                    fclose(indexRead);
                    fclose(indexTemp);

                    // remove current index.txt and replace with temp file with all accounts except the file to be deleted
                    remove("database/index.txt");
                    rename("database/temp_index.txt", "database/index.txt");
                    
                    printf("Account deleted successfully.\n");
                    return; 
                } else {
                    printf("Error deleting account.\n");
                    return;
                }
            } else if (confirm == 'n') {
                printf("Account deletion canceled.\n");
                return;
            } else {
                printf("Input error. Please input 'y' or 'n' to confirm deletion. \n");
            }
        }
    }
}


// --- 3/4. Deposit / Withdraw ---
void updateBalance(char operation,int amount, int accountNumber, const char *receiverType) {
    char filename[128];
    sprintf(filename, "database/%d.txt", accountNumber);

    FILE *accFile = fopen(filename, "r+"); // read and write to file
    if (!accFile) {
        printf("Account not found.\n");
        return;
    }

    // read file
    fscanf(accFile, "Name: %[^\n]\n", acc.name);
    fscanf(accFile, "ID: %12s\n", acc.ID);
    fscanf(accFile, "Account Number: %d\n", &acc.accountNumber);
    fscanf(accFile, "Account Type: %[^\n]\n", acc.type);
    fscanf(accFile, "PIN: %4s\n", acc.pin);
    fscanf(accFile, "Balance: %f\n", &acc.balance);

    float fee = 0.0;
    if (operation == '+') {
        // validate deposit amount between 0 and 50000
        if (amount > 0 && amount <= 50000) {
            acc.balance += amount;
            printf("Deposit successful. ");
        } else{
            printf("Please input between RM0 and RM50,000 only.");
            fclose(accFile);
            return;
        }
    } else if (operation == '-') {
        // validate when remittance, percentage fee based on transfer accounts 
        if (receiverType != NULL) {
            if (strcmp(acc.type, "Savings") == 0 && strcmp(receiverType, "Current") == 0) {
                fee = 0.02; // 2% fee
            } else if (strcmp(acc.type, "Current") == 0 && strcmp(receiverType, "Savings") == 0) {
                fee = 0.03; // 3% fee
            } else {
                printf("Transfer error. Transfers only allowed between different account types.\n");
                printf("Savings → Current (2%% fee) or Current → Savings (3%% fee).\n");
                printf("Same account type transfers are not permitted. \n");
                fclose(accFile);
                return;
            }
        }
        float totalAmount = amount + (amount * fee);
        if (totalAmount > acc.balance) {
            printf("Insufficient balance including remittance fee.\n");
            fclose(accFile);
            return;
        }

        acc.balance -= totalAmount;
        if (fee > 0) {
            printf("A remittance fee of %.2f%% has been applied.\n", fee * 100);
        }
        printf("Withdrawal/Transfer successful. \n");
    }

    // Go back to start of file and rewrite
    fseek(accFile, 0, SEEK_SET);
    // write to file
    fprintf(accFile, "Name: %s\n", acc.name);
    fprintf(accFile, "ID: %s\n", acc.ID);
    fprintf(accFile, "Account Number: %d\n", acc.accountNumber);
    fprintf(accFile, "Account Type: %s\n", acc.type);
    fprintf(accFile, "PIN: %s\n", acc.pin);
    fprintf(accFile, "Balance: %.2f\n", acc.balance);
    fclose(accFile);

    printf("New balance: %.2f\n", acc.balance);
}


// --- 3,4,5. Deposit, Withdraw, Remittance (using updateBalance) ---
void deposit() {
    int accountNumber;
    if (!verifyAccount(0, &accountNumber)) return; // if pin is wrong, return

    int amount;
    printf("How much would you like to deposit? ");
    scanf("%d", &amount);
    updateBalance('+', amount, accountNumber, NULL);
}

void withdraw() {
    int accountNumber;
    if (!verifyAccount(0, &accountNumber)) return; // if pin is wrong, return
    
    int amount;
    printf("How much would you like to withdraw? ");
    scanf("%d", &amount);
    updateBalance('-', amount, accountNumber, NULL);
}

void remittance() {
    int senderAccount;
    if (!verifyAccount(0, &senderAccount)) return; // if pin is wrong, return

    int receiverAccount;
    printf("Enter recipient account number: ");
    scanf("%d", &receiverAccount);
    if (!isAccountNumberInIndex(receiverAccount)) return; // only need verify account number, not pin or ID

    int amount;
    printf("How much would you like to transfer? ");
    scanf("%d", &amount);

    // get receiver type
    char filename[128];
    char receiverType[10];

    sprintf(filename, "database/%d.txt", receiverAccount);
    FILE *file; 
    file = fopen(filename, "r");
    if (!file) {
        printf("Recipient account file not found.\n");
        return;
    }

    // read only receiverType in file
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "Account Type:") != NULL) {
            sscanf(line, "Account Type: %[^\n]", receiverType);
            break;
        }
    }
    fclose(file);

    updateBalance('-', amount, senderAccount, receiverType);
    updateBalance('+', amount, receiverAccount, NULL);

    printf("Transfer Succesful!!");
}

int main() {
    time_t t = time(NULL);
    printf("--- Banking Sytem ---\n");
    printf("Session start: %s\n", ctime(&t));
    printf("No. Accounts Loaded: %d\n\n", countAccounts());

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
            deleteAccount();
            logTransaction("delete account");
        } 
        else if (strcmp(choice, "3") == 0 || strcmp(choice, "deposit") == 0) {
            printf("Depositing...\n");
            deposit();
            logTransaction("deposit");
        } 
        else if (strcmp(choice, "4") == 0 || strcmp(choice, "withdraw") == 0) {
            printf("Withdrawing...\n");
            withdraw();
            logTransaction("withdrawal");
        } else if (strcmp(choice, "5") == 0 || strcmp(choice, "remittance") == 0) {
            printf("Remitting funds...");
            remittance();
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