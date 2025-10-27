/*
Description:
- This is version 1 of the banking system. Fully functional with all core functions required (create account, delete account, deposit, withdraw, remittance)
- This version focuses on functionality with little formatting and styling. 
- For a more beautified and improved version, go to the file 'v2/main.c'
- Total time spent on both versions: 30+ hours
- Only native C methods listed by the assignment were used.
*/

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

// Better input function that handles buffer clearing
void getInput(const char* prompt, char* input, int inputSize) {
    printf("%s", prompt);
    
    // read input
    int i = 0;
    char ch;
    // loop while index is < input size - 1 (leave space for \0) and until user presses enter or End of File (EOF)
    while (i < inputSize - 1 && (ch = getchar()) != '\n' && ch != EOF) {
        // store character in input string array & move pointer to next index
        input[i] = ch;
        i++;
    }
    // append to input string array to terminate
    input[i] = '\0';

    // if input too long, clear input buffer (remaining chars) 
    if (ch != '\n' && ch != EOF) {
        while ((ch = getchar()) != '\n' && ch != EOF);
    }
}

// exit to menu by pressing 'q' or 'Q'
int exitToMenu(const char* input) {
    if (strcmp(input, "q") == 0 || strcmp(input, "Q") == 0) {
        return 1;
    }
    return 0;
}

int countAccounts() {
    FILE *indexFile = fopen("database/index.txt", "r");
    if (!indexFile) return 0;

    int count = 0, number;
    while (1) {
        int result = fscanf(indexFile, "%d", &number);
        if (result != 1) {
            break; // when no more numbers exit
        }
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
    // remove newline from buffer to print on same line
    timeStr[strcspn(timeStr, "\n")] = 0; // finds position of \\n and replaces with \\0 to end string

    fprintf(transactionLog, "[%s] %s\n", timeStr, message);
    fclose(transactionLog);
}

// check if account number exists in index.txt
int isAccountNumberInIndex(int accNum) {
    FILE *indexFile = fopen("database/index.txt", "r");
    if (!indexFile) {
        printf("Error: couldn't open index file.\n");
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
        char accNumInput[10];
        int accNum;
        char pinInput[5];
        char idInput[5];

        int accountFound = 1;
        // verify account number
        while (accountFound) {
            getInput("Enter your account number: ", accNumInput, sizeof(accNumInput));
            if (exitToMenu(accNumInput)) {
                return 0;
            }
            accNum = atoi(accNumInput); // convert string to integer

            if (!isAccountNumberInIndex(accNum)) {
                printf("Account number not found. Please try again.\n");
            } else {
                accountFound = 0;
                *returnAccountNumber = accNum;  // store converted integer
            }
        }

        // get data of account number inputted from file to compare
        char filename[128];
        sprintf(filename, "database/%d.txt", accNum); // format account number for file directory

        FILE *accFile;
        accFile = fopen(filename, "r");
        if (!accFile) {
            printf("Account not found.\n");
            return 0;
        }

        char storedName[100], storedPIN[5], storedID[13], typeStr[20];
        int storedAccNum;
        float balance;

        // read and store account data
        fscanf(accFile, "Name: %[^\n]\n", storedName); // long string %[^\n] reads until newline
        fscanf(accFile, "ID: %12s\n", storedID);
        fscanf(accFile, "Account Number: %d\n", &storedAccNum);
        fscanf(accFile, "Account Type: %[^\n]\n", typeStr); // long string %[^\n] reads until newline
        fscanf(accFile, "PIN: %4s\n", storedPIN);
        fscanf(accFile, "Balance: %f\n", &balance);

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
                getInput("Enter last 4 characters of your ID: ", idInput, sizeof(idInput));
                if (exitToMenu(idInput)) {
                    return 0;
                }

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
            getInput("Enter your 4-digit PIN: ", pinInput, sizeof(pinInput));
            if (exitToMenu(pinInput)) {
                return 0;
            }

            // compare pin inputted with stored pin
            if (strcmp(pinInput, storedPIN) == 0) {
                *returnAccountNumber = accNum; // if pins are same (correct), return account number for use
                printf("Account verified: %d", storedAccNum);
                return 1;
            } else {
                attemptsLeft--; // if pins are not same (wrong), decrement attempt and exit if no more attempts left
                if (attemptsLeft == 0) {
                    printf("You have run out of attempts.\n");
                    return 0;
                } else {
                    printf("Incorrect PIN. You have: %d attempts left.\n", attemptsLeft);
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
        printf("Error: couldn't open index file.\n");
        return;
    }
    fprintf(indexFile, "%d\n", accountNumber);
    fclose(indexFile);
}

void createAccount() {
    printf("\n=== Create New Account ===\n");
    getInput("Full name: ", acc.name, sizeof(acc.name));
    if (exitToMenu(acc.name)) {
        return;
    }

    int valid = 0;
    // check if ID inputted is less than 13 char & is integers
    while (!valid) {
        getInput("Identification Number (ID): ", acc.ID, sizeof(acc.ID));
        if (exitToMenu(acc.ID)) {
            return;
        }

        valid = 1;
        int idLength = strlen(acc.ID);
        for (int i = 0; i < idLength; i++) {
            // check if acc Id is digit
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
        getInput("Account type (0 = savings, 1 = current): ", typeInput, sizeof(typeInput));
        if (exitToMenu(typeInput)) {
            return;
        }

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
    char pin1Input[10], pin2Input[10];
    while (running) {
        getInput("Set 4-digit PIN: ", pin1Input, sizeof(pin1Input));
        if (exitToMenu(pin1Input)) {
            return;
        }

        if (sscanf(pin1Input, "%d", &pin1) != 1) {
            printf("Invalid input. Please enter digits only.\n");
            continue;
        }

        if (pin1 < 1000 || pin1 > 9999) {
            printf("Invalid PIN. Must be 4 digits.\n");
            continue;
        }

        getInput("Re-enter PIN to confirm: ", pin2Input, sizeof(pin2Input));
        if (exitToMenu(pin2Input)) {
            return;
        }

        if (sscanf(pin2Input, "%d", &pin2) != 1) {
            printf("Invalid input. Please enter digits only.\n");
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
        int min = 1000000; // 7 digits
        int max = 999999999; // 9 digits
        accountNumber = min + rand() % (max + 1 - min);;
    } while (isAccountNumberInIndex(accountNumber)); // keep generating a random number until it is not in index.txt

    acc.accountNumber = accountNumber;
    saveAccountNumber(accountNumber);

    // create filename based on account number
    char filename[100];
    sprintf(filename, "database/%d.txt", acc.accountNumber); // format to account number in file directory

    FILE *accFile = fopen(filename, "w");
    if (accFile == NULL) {
        printf("Error. File is missing. Failed to create new account.\n");
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

    printf("Account created successfully!\n");
}


// --- 2. delete functions ---
void getAccounts() {    
    FILE *indexFile;
    indexFile = fopen("database/index.txt", "r");
    if (indexFile == NULL) {
        printf("Error: couldn't open index file to retrieve account numbers.\n");
        return;
    }
    
    printf("[  Saved Accounts List  ]\n");
    char line[128];
    while (fgets(line, sizeof(line), indexFile) != NULL) {
        line[strcspn(line, "\n")] = 0;
        printf("%s\n", line);
    }
    fclose(indexFile);

    // get number of accounts loaded
    printf("No. of Accounts Loaded: %d\n", countAccounts());
}

void deleteAccount() {
    printf("\n=== Delete Account ===\n");
    // print all account numbers in database
    getAccounts();

    int accountNumber;

    // confirm deletion
    if (verifyAccount(1, &accountNumber)) {
        char confirm[2];
        getInput("Are you sure you want to delete your account? (y/n): ", confirm, sizeof(confirm));
            
        if (tolower(confirm[0]) == 'y') {
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
                // validation checks
                if (remove("database/index.txt") != 0) {
                    printf("Error: Could not remove old index file.\n");
                    remove("database/temp_index.txt");
                    return;
                }

                if (rename("database/temp_index.txt", "database/index.txt") != 0) {
                    printf("Error: Could not rename temporary index file.\n");
                    return;
                }
                
                printf("Account deleted successfully\n");
                return; 
            } else {
                printf("Error deleting account.\n");
                return;
            }
        } else if (tolower(confirm[0]) == 'n') {
            printf("Account deleted canceled.\n");
            return;
        } else {
            printf("Input error. Please input 'y' or 'n' to confirm deletion.\n");
        }
    }
}

// getAccountBalance for withdraw and remittance
float getAccountBalance(int accountNumber) {
    char filename[128];
    sprintf(filename, "database/%d.txt", accountNumber);
    
    FILE *accFile = fopen(filename, "r");
    if (!accFile) return -1;

    char line[256];
    float balance = -1;
    
    while (fgets(line, sizeof(line), accFile)) {
        if (strstr(line, "Balance:") != NULL) {
            sscanf(line, "Balance: %f", &balance);
            break;
        }
    }
    
    fclose(accFile);
    return balance;
}

// --- 3/4. Deposit / Withdraw ---
int updateBalance(char operation, float amount, int accountNumber, const char *receiverType) {
    char filename[128];
    sprintf(filename, "database/%d.txt", accountNumber);

    FILE *accFile = fopen(filename, "r+"); // read and write to file
    if (!accFile) {
        printf("Account not found.\n");
        return 0;
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
            printf("Deposit successful.\n");
        } else{
            printf("Please input between RM 0 and RM 50,000 only\n");
            fclose(accFile);
            return 0;
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
                printf("Savings --> Current (2%% fee) or Current --> Savings (3%% fee).\n");
                printf("Same account type transfers are not permitted.\n");
                fclose(accFile);
                return 0; // fail
            }
        }
        float totalAmount = amount + (amount * fee);
        if (totalAmount > acc.balance) {
            printf("Insufficient balance including remittance fee\n");
            fclose(accFile);
            return 0;
        }

        acc.balance -= totalAmount;
        if (fee > 0) {
            printf("A remittance fee of %.2f%% has been applied.\n", fee * 100);
        }
        printf("Withdrawal/Transfer successful.\n");
    }

    // Go back to start of file
    rewind(accFile);
    // rewrite accFile info with new balance
    fprintf(accFile, "Name: %s\n", acc.name);
    fprintf(accFile, "ID: %s\n", acc.ID);
    fprintf(accFile, "Account Number: %d\n", acc.accountNumber);
    fprintf(accFile, "Account Type: %s\n", acc.type);
    fprintf(accFile, "PIN: %s\n", acc.pin);
    fprintf(accFile, "Balance: %.2f\n", acc.balance);
    fclose(accFile);

    // only show account balance if withdrawing money from own account (for deposit own account, show account balance locally)
    // so that receiever account balance is not shown when remitting
    if (operation == '-') {
        printf("Your new account balance is: %.2f\n", acc.balance);
    }

    return 1;
}


// --- 3,4,5. Deposit, Withdraw, Remittance (using updateBalance) ---
void deposit() {
    printf("\n=== Deposit Amount ===\n");
    getAccounts();

    int accountNumber;
    if (!verifyAccount(0, &accountNumber)) return; // if pin is wrong, return

    char amountInput[10];
    getInput("How much would you like to deposit? ", amountInput, sizeof(amountInput));
    float amount = atof(amountInput); // convert ascii to float

    // if updateBalance successful (1) then print current balance
    if (updateBalance('+', amount, accountNumber, NULL)) {
        float newBalance = getAccountBalance(accountNumber);
        if (newBalance >= 0) {
            printf("Current Balance: RM%.2f\n", newBalance);
        }
    }
}

void withdraw() {
    printf("\n=== Withdraw Amount ===\n");
    getAccounts();

    int accountNumber;
    if (!verifyAccount(0, &accountNumber)) return; // if pin is wrong, return

    // get current account balance
    float currentBalance = getAccountBalance(accountNumber);
    if (currentBalance >= 0) {
        printf("Current Balance: RM%.2f\n", currentBalance);
    }
    
    char amountInput[10];
    getInput("How much would you like to withdraw? ", amountInput, sizeof(amountInput));
    float amount = atof(amountInput);
    updateBalance('-', amount, accountNumber, NULL); // update balance and print new acc balance
}

void remittance() {
    printf("\n=== Transfer Amount ===\n");
    getAccounts();

    int senderAccount;
    if (!verifyAccount(0, &senderAccount)) return; // if pin is wrong, return

    char receiverInput[10];
    getInput("Enter recipient account number: ", receiverInput, sizeof(receiverInput));
    int receiverAccount = atoi(receiverInput);
    if (!isAccountNumberInIndex(receiverAccount)) return; // only need verify account number, not pin or ID

    // get current account balance
    float currentBalance = getAccountBalance(senderAccount);
    if (currentBalance >= 0) {
        printf("Your current balance is: RM%.2f\n", currentBalance);
    }

    char amountInput[10];
    getInput("How much would you like to transfer? ", amountInput, sizeof(amountInput));
    float amount = atof(amountInput); // convert to float

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

    // validate updateBalance and pass receiverType to compare with senderType for remittance fee
    if (updateBalance('-', amount, senderAccount, receiverType)) {
        // only update receiver account if sender account was successful updated
        updateBalance('+', amount, receiverAccount, NULL);
        printf("Transfer completed successfully!\n");
    } else {
        printf("Transfer failed. No changes were made.\n");
    }
}

int main() {
    time_t t = time(NULL);
    char* timeStr = ctime(&t);
    timeStr[strcspn(timeStr, "\n")] = 0; // remove newline

    logTransaction("Session Start");
    printf("\n=== Welcome to the official Bank System! ===\n");
    printf("\nSession start: %s\n", timeStr);
    printf("No. of Accounts Loaded: %d\n", countAccounts());
    char choice[20];

    int running = 1;
    while (running) {
        printf("\n=== Main Menu ===\n");
        printf("Please choose the following (1-6):\n");
        printf("1. Create Account\n");
        printf("2. Delete Account\n");
        printf("3. Deposit\n");
        printf("4. Withdraw\n");
        printf("5. Remittance\n");
        printf("6. Exit\n");
        printf("Tip: Press 'q' to exit and return to main menu.\n");

        getInput("Select Option: ", choice, sizeof(choice));
        if (exitToMenu(choice)) {
            continue; // restart menu
        }

        // convert choice to lowercase
        int i = 0;
        while (choice[i] != '\0') {
            choice[i] = tolower(choice[i]);  // turn each character into lowercase
            i++;
        }

        if (strcmp(choice, "1") == 0 || strcmp(choice, "create") == 0) {
            createAccount();
            logTransaction("Create account");
        } 
        else if (strcmp(choice, "2") == 0 || strcmp(choice, "delete") == 0) {
            deleteAccount();
            logTransaction("Delete account");
        } 
        else if (strcmp(choice, "3") == 0 || strcmp(choice, "deposit") == 0) {
            deposit();
            logTransaction("Deposit");
        } 
        else if (strcmp(choice, "4") == 0 || strcmp(choice, "withdraw") == 0) {
            withdraw();
            logTransaction("Withdrawal");
        } else if (strcmp(choice, "5") == 0 || strcmp(choice, "remittance") == 0) {
            remittance();
            logTransaction("Remittance");
        } else if (strcmp(choice, "6") == 0 || strcmp(choice, "exit") == 0) {
            printf("Thank you for using our service. Please come again next time... BYE!\n");
            logTransaction("Session ended");
            break;
        } 
        else {
            printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}