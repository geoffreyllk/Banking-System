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

typedef enum { UITop, UIMiddle, UIBottom, UIBorder } UIPositionY;
typedef enum { UILeft, UICenter, UIRight } UIPositionX;

// size of UI e.g. 50 characters wide
const int UIWidth = 50;
void printUI(const char* text, UIPositionY posY, UIPositionX posX) {
    int textLength = strlen(text);

    switch (posY) {
        // if text at top, print seperator 
        // ' __________________'
        // '/                  \'
        case UITop:
            printf(" ");
            // print seperator - 2 (excluding / and \ at edges)
            for (int i = 0; i < UIWidth - 2; i++) printf("_");
            printf(" \n");
            printf("/");
            for (int i = 0; i < UIWidth - 2; i++) printf(" ");
            printf("\\\n");
            break;
        // if text at bottom, print seperator '\__________________/'
        case UIBottom:
            printf("\\");
            // print seperator - 2 (excluding '/' and '\' at edges)
            for (int i = 0; i < UIWidth - 2; i++) printf("_");
            printf("/\n");
            break;
        // if text in middle, print '|' at the edges
        case UIMiddle:
            printf("|");
            
            // Xposition of text, left center right
            if (posX == UICenter) {
                // padding is UIWidth - text - 2(border '|' at edges) and divide by 2 for left and right
                int padding = (UIWidth - textLength - 2) / 2;
                // print padding e.g. "|    hi    |"
                for (int i = 0; i < padding; i++) printf(" ");
                printf("%s", text);
                for (int i = 0; i < padding; i++) printf(" ");

                // if total padding is odd, add one extra space on the right
                if ( ((UIWidth - textLength - 2) % 2) != 0) {
                    printf(" ");
                }
            }
            else if (posX == UILeft) {
                // if left, print '|  text          |'
                printf("  %s", text);
                // - 4 to include the double spacing before text '|  text'
                for (int i = 0; i < UIWidth - textLength - 4; i++) printf(" ");
            }
            else if (posX == UIRight) {
                // if right, print '|          text  |'
                for (int i = 0; i < UIWidth - textLength - 4; i++) printf(" ");
                // - 3 to include the double spacing after text 'text  |'
                printf("%s  ", text);
            }
            
            // close border and go next line
            printf("|\n");
            break;
        // if border then print '| ----------- |' with one spacing beside left and right borders (border can be customised e.g. '-' or '_ or '=' )
        case UIBorder:
            printf("| ");
            for (int i = 0; i < UIWidth - 4; i++) printf(text);
            printf(" |\n");
    }
}

// for inputs by user, printing a complete user prompt inside UI border
void printInput(const char* prompt, char* input, int inputSize) {
    int promptLength = strlen(prompt);
    printf("|  %s", prompt);
    
    for (int i = 0; i < UIWidth - promptLength - 6; i++) printf(" ");
    printf("  |\n");

    // move cursor
    printf("\033[1A"); // move up by 1 line
    printf("\033[%dC", promptLength + 3); // move to input position

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
}

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
        printUI("Error: couldn’t open index file.", UIMiddle, UILeft);
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
            printInput("Enter your account number: ", accNumInput, sizeof(accNumInput));
            accNum = atoi(accNumInput); // convert string to integer

            if (!isAccountNumberInIndex(accNum)) {
                printUI("Account number not found. Please try again.", UIMiddle, UILeft);
            } else {
                accountFound = 0;
                *returnAccountNumber = accNum;  // store converted integer
            }
        }

        // get data of account number inputted from file to compare
        char filename[128];
        sprintf(filename, "database/%d.txt", accNum);

        FILE *accFile;
        accFile = fopen(filename, "r");
        if (!accFile) {
            printUI("Account not found.", UIMiddle, UILeft);
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
                printInput("Enter last 4 characters of your ID: ", idInput, sizeof(idInput));

                // compare strings
                if (strcmp(idInput, last4IDs) != 0) {
                    printUI("Incorrect ID. Try again.", UIMiddle, UILeft);
                } else {
                    idFound = 0;
                }
            }
        }

        // verify pin with 4 attempts
        int attemptsLeft = 4;
        while (attemptsLeft > 0) {
            printInput("Enter your 4-digit PIN: ", pinInput, sizeof(pinInput));

            // compare pin inputted with stored pin
            if (strcmp(pinInput, storedPIN) == 0) {
                *returnAccountNumber = accNum; // if pins are same (correct), return account number for use
                return 1;
            } else {
                attemptsLeft--; // if pins are not same (wrong), decrement attempt and exit if no more attempts left
                if (attemptsLeft == 0) {
                    printUI("You have run out of attempts. Returning to main menu.", UIMiddle, UILeft);
                    return 0;
                } else {
                    char attemptsMsg[50];
                    sprintf(attemptsMsg, "Incorrect PIN. You have: %d attempts left.", attemptsLeft);
                    printUI(attemptsMsg, UIMiddle, UILeft);
                }
            }
        }
    }
    return 0;
}

// time delay
void delay(int number_of_seconds)
{	
	int milli_seconds = 1000 * number_of_seconds; // convert seconds to milliseconds
	clock_t start_time = clock();	
	while (clock() < start_time + milli_seconds); // looping until not time
}

// --- 1. create account functions ---

void saveAccountNumber(int accountNumber) {
    FILE *indexFile;
    indexFile = fopen("database/index.txt", "a");
    if (indexFile == NULL) {
        printUI("Error: couldn’t open index file.", UIMiddle, UILeft);
        return;
    }
    fprintf(indexFile, "%d\n", accountNumber);
    fclose(indexFile);
}

void createAccount() {
    printUI("", UITop, UICenter);
    printUI("Please fill in the following", UIMiddle, UILeft);
    printInput("Full name: ", acc.name, sizeof(acc.name));

    int valid = 0;
    // check if ID inputted is less than 13 char & is integers
    while (!valid) {
        printInput("Identification Number (ID): ", acc.ID, sizeof(acc.ID));

        valid = 1;
        for (size_t i = 0; i < strlen(acc.ID); i++) {
            // check if acc Id is digit
            if (!isdigit(acc.ID[i])) {
                valid = 0;
                printUI("Invalid ID. Only numbers allowed.", UIMiddle, UILeft);
                break;
            }
        }
        if (valid && (strlen(acc.ID) < 8 || strlen(acc.ID) > 12)) {
            valid = 0;
            printUI("Invalid ID length. Must be 8-12 digits.", UIMiddle, UILeft);
        }
    }

    int running = 1;
    // validate 0 or 1 for account type
    while (running) {
        char typeInput[10];
        printInput("Account type (0 = savings, 1 = current): ", typeInput, sizeof(typeInput));

        if (strcmp(typeInput, "0") == 0) {
            strcpy(acc.type, "Savings");
            break;
        } else if (strcmp(typeInput, "1") == 0) {
            strcpy(acc.type, "Current");
            break;
        } else {
            printUI("Invalid type. Please enter 0 or 1 only.", UIMiddle, UILeft);
        }
    }

    // verify password via double entry
    int pin1, pin2;
    char pin1Input[10], pin2Input[10];
    while (running) {
        printInput("Set 4-digit PIN: ", pin1Input, sizeof(pin1Input));
        if (sscanf(pin1Input, "%d", &pin1) != 1) {
            printUI("Invalid input. Please enter digits only.", UIMiddle, UILeft);
            continue;
        }

        if (pin1 < 1000 || pin1 > 9999) {
            printUI("Invalid PIN. Must be 4 digits.", UIMiddle, UILeft);
            continue;
        }

        printInput("Re-enter PIN to confirm: ", pin2Input, sizeof(pin2Input));
        if (sscanf(pin2Input, "%d", &pin2) != 1) {
            printUI("Invalid input. Please enter digits only.", UIMiddle, UILeft);
            continue;
        }

        if (pin1 == pin2) {
            sprintf(acc.pin, "%04d", pin1);
            break;
        } else {
            printUI("PINs do not match. Please try again.", UIMiddle, UILeft);
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
        printUI("Error: could not create account file.", UIMiddle, UILeft);
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

    printUI("Account created successfully!", UIMiddle, UICenter);
}


// --- 2. delete functions ---
void getAccounts() {
    FILE *indexFile;
    indexFile = fopen("database/index.txt", "r");
    if (indexFile == NULL) {
        printUI("Error: couldn’t open index file to retrieve account numbers.", UIMiddle, UILeft);
        return;
    }
    printUI("Saved Accounts:", UIMiddle, UILeft);
    char line[128];
    while (fgets(line, sizeof(line), indexFile) != NULL) {
        line[strcspn(line, "\n")] = 0;
        printUI(line, UIMiddle, UILeft);
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
            char confirm[2];
            printInput("Are you sure you want to delete your account? (y/n): ", confirm, sizeof(confirm));

            if (confirm[0] == 'y' || confirm[0] == 'Y') {
                // create filename string of account number e.g. 'database/1234567.txt'
                char filename[128];
                sprintf(filename, "database/%d.txt", accountNumber);

                FILE *accFile;
                accFile = fopen(filename, "r");
                // check if account file exists, if not return not found
                if (!accFile) {
                    printUI("Account not found.", UIMiddle, UILeft);
                    return;
                }
                fclose(accFile);

                
                if (remove(filename) == 0) {
                    // since cannot directly delete files in c, read all account numbers NOT to be deleted, and write them to a different temp file
                    FILE *indexRead = fopen("database/index.txt", "r");
                    // error check
                    if (!indexRead) {
                        printUI("Error: Could not open index.txt for reading.", UIMiddle, UILeft);
                        return;
                    }

                    FILE *indexTemp = fopen("database/temp_index.txt", "w");
                    if (!indexTemp) {
                        printUI("Error: Could not create temporary index file.", UIMiddle, UILeft);
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
                    
                    printUI("Account deleted successfully.", UIMiddle, UICenter);
                    return; 
                } else {
                    printUI("Error deleting account.", UIMiddle, UILeft);
                    return;
                }
            } else if (confirm[0] == 'n' || confirm[0] == 'N') {
                printUI("Account deletion canceled.", UIMiddle, UICenter);
                return;
            } else {
                printUI("Input error. Please input 'y' or 'n' to confirm deletion.", UIMiddle, UILeft);
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
        printUI("Account not found.", UIMiddle, UILeft);
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
            printUI("Deposit successful.", UIMiddle, UILeft);
        } else{
            printUI("Please input between RM0 and RM50,000 only.", UIMiddle, UILeft);
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
                printUI("Transfer error. Transfers only allowed between different account types.", UIMiddle, UILeft);
                printUI("Savings → Current (2% fee) or Current → Savings (3% fee).", UIMiddle, UILeft);
                printUI("Same account type transfers are not permitted.", UIMiddle, UILeft);
                fclose(accFile);
                return;
            }
        }
        float totalAmount = amount + (amount * fee);
        if (totalAmount > acc.balance) {
            printUI("Insufficient balance including remittance fee.", UIMiddle, UILeft);
            fclose(accFile);
            return;
        }

        acc.balance -= totalAmount;
        if (fee > 0) {
            char feeMsg[50];
            sprintf(feeMsg, "A remittance fee of %.2f%% has been applied.", fee * 100);
            printUI(feeMsg, UIMiddle, UILeft);
        }
        printUI("Withdrawal/Transfer successful.", UIMiddle, UILeft);
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

    char balanceMsg[50];
    sprintf(balanceMsg, "New balance: %.2f", acc.balance);
    printUI(balanceMsg, UIMiddle, UILeft);
}


// --- 3,4,5. Deposit, Withdraw, Remittance (using updateBalance) ---
void deposit() {
    int accountNumber;
    if (!verifyAccount(0, &accountNumber)) return; // if pin is wrong, return

    char amountInput[10];
    printInput("How much would you like to deposit? ", amountInput, sizeof(amountInput));
    int amount = atoi(amountInput);
    updateBalance('+', amount, accountNumber, NULL);
}

void withdraw() {
    int accountNumber;
    if (!verifyAccount(0, &accountNumber)) return; // if pin is wrong, return
    
    char amountInput[10];
    printInput("How much would you like to withdraw? ", amountInput, sizeof(amountInput));
    int amount = atoi(amountInput);
    updateBalance('-', amount, accountNumber, NULL);
}

void remittance() {
    int senderAccount;
    if (!verifyAccount(0, &senderAccount)) return; // if pin is wrong, return

    char receiverInput[10];
    printInput("Enter recipient account number: ", receiverInput, sizeof(receiverInput));
    int receiverAccount = atoi(receiverInput);
    if (!isAccountNumberInIndex(receiverAccount)) return; // only need verify account number, not pin or ID

    char amountInput[10];
    printInput("How much would you like to transfer? ", amountInput, sizeof(amountInput));
    int amount = atoi(amountInput);

    // get receiver type
    char filename[128];
    char receiverType[10];

    sprintf(filename, "database/%d.txt", receiverAccount);
    FILE *file; 
    file = fopen(filename, "r");
    if (!file) {
        printUI("Recipient account file not found.", UIMiddle, UILeft);
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

    printUI("Transfer Successful!", UIMiddle, UICenter);
}

// small helper to print loading texts in main e.g. 'Depositing...'
void printLoad(const char* text) {
    printUI(text, UIMiddle, UILeft);  
    printUI("", UIBottom, UICenter);
    delay(2);
    system("cls");
}

int main() {
    time_t t = time(NULL);
    char* timeStr = ctime(&t);
    timeStr[strcspn(timeStr, "\n")] = 0; // remove newline

    logTransaction("Session Start");
    printUI("", UITop, UICenter);
    printUI("Banking System", UIMiddle, UICenter);  
    printUI("", UIMiddle, UICenter);  
    
    char sessionText[100];
    sprintf(sessionText, "Session start: %s", timeStr);
    printUI(sessionText, UIMiddle, UILeft);
    
    char accountsText[50];
    sprintf(accountsText, "No. Accounts Loaded: %d", countAccounts());
    printUI(accountsText, UIMiddle, UILeft);
    
    printUI("-", UIBorder, UICenter);
    printUI("", UIMiddle, UICenter);  
    char choice[20];

    int running = 1;
    while (running) {
        printUI("Please choose the following (1-6): ", UIMiddle, UILeft);  
        printUI("1. Create Account", UIMiddle, UILeft);  
        printUI("2. Delete Account", UIMiddle, UILeft);  
        printUI("3. Deposit", UIMiddle, UILeft);  
        printUI("4. Withdraw", UIMiddle, UILeft);  
        printUI("5. Remittance", UIMiddle, UILeft);  
        printUI("6. Exit", UIMiddle, UILeft);
        printUI("-", UIBorder, UICenter);

        // get input of char choice, with print 'Select Option: '
        printInput("Select Option: ", choice, sizeof(choice)); 
        printUI("-", UIBorder, UICenter); 

        if (strcmp(choice, "1") == 0 || strcmp(choice, "create") == 0) {
            printLoad("Creating account...");  
            createAccount();
            logTransaction("Create account");
        } 
        else if (strcmp(choice, "2") == 0 || strcmp(choice, "delete") == 0) {
            printLoad("Deleting account...");
            deleteAccount();
            logTransaction("Delete account");
        } 
        else if (strcmp(choice, "3") == 0 || strcmp(choice, "deposit") == 0) {
            printLoad("Depositing...");
            deposit();
            logTransaction("Deposit");
        } 
        else if (strcmp(choice, "4") == 0 || strcmp(choice, "withdraw") == 0) {
            printLoad("Withdrawing...");
            withdraw();
            logTransaction("Withdrawal");
        } else if (strcmp(choice, "5") == 0 || strcmp(choice, "remittance") == 0) {
            printLoad("Remitting funds...");
            remittance();
            logTransaction("Remittance");
        } else if (strcmp(choice, "6") == 0 || strcmp(choice, "exit") == 0) {
            printLoad("Thank you for using our service. Please come again next time... BYE!");
            logTransaction("Session ended");
            break;
        } 
        else {
            printUI("Invalid choice. Please try again.", UIMiddle, UILeft);
        }
    }

    return 0;
}