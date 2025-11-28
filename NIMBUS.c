#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAINTENANCE_INTERVAL_KM 10000
#define MAINTENANCE_INTERVAL_DAYS 180
#define LOG_FILE "maintenance_log.txt"
#define DATA_FILE "fleet_data.csv"

#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define RESET   "\033[0m"

typedef struct {
    int bus_no;
    int last_service_mileage;
    int current_mileage;
    char last_service_date[11];
    int next_due_mileage;
    int days_since_service;
    int due_in_days;
    int overdue_flag;
} Bus;

Bus *fleet = NULL;
int fleet_size = 0;

void addBus();
void displayFleet();
void updateMileage();
void predictMaintenance(Bus *b);
int dateToDays(char *date);
int calculateDaysDifference(char *date);
void checkOverdue();
void logOverdue(Bus *b);
void saveToFile();
void loadFromFile();
void searchBus();
void sortFleetByDue();
int isValidDate(char *date);
int askInt();
void header();

void header() {
    printf(BLUE "\n===========================================================\n" RESET);
    printf(BLUE "            ADVANCED BUS FLEET MAINTENANCE TRACKER\n" RESET);
    printf(BLUE "===========================================================\n" RESET);
}

int askInt() {
    int x;
    while (scanf("%d", &x) != 1) {
        printf(RED "Invalid input. Enter an integer: " RESET);
        while (getchar() != '\n');
    }
    return x;
}

int isValidDate(char *d) {
    if (strlen(d) != 10) return 0;
    if (d[2] != '-' || d[5] != '-') return 0;
    int dd, mm, yy;
    sscanf(d, "%d-%d-%d", &dd, &mm, &yy);
    if (dd < 1 || dd > 31) return 0;
    if (mm < 1 || mm > 12) return 0;
    if (yy < 1900 || yy > 2100) return 0;
    return 1;
}

int dateToDays(char *date) {
    int d, m, y;
    sscanf(date, "%d-%d-%d", &d, &m, &y);
    return y * 365 + m * 30 + d;
}

int calculateDaysDifference(char *date) {
    char today[11] = "01-12-2025";
    return dateToDays(today) - dateToDays(date);
}

void predictMaintenance(Bus *b) {
    b->next_due_mileage = b->last_service_mileage + MAINTENANCE_INTERVAL_KM;
    b->days_since_service = calculateDaysDifference(b->last_service_date);
    b->due_in_days = MAINTENANCE_INTERVAL_DAYS - b->days_since_service;
    b->overdue_flag =
        (b->current_mileage >= b->next_due_mileage) ||
        (b->days_since_service >= MAINTENANCE_INTERVAL_DAYS);
}

void addBus() {
    fleet = realloc(fleet, (fleet_size + 1) * sizeof(Bus));
    Bus *b = &fleet[fleet_size];

    printf("\nEnter Bus Number: ");
    b->bus_no = askInt();

    printf("Enter Last Service Mileage: ");
    b->last_service_mileage = askInt();

    printf("Enter Current Mileage: ");
    b->current_mileage = askInt();

    do {
        printf("Enter Last Service Date (DD-MM-YYYY): ");
        scanf("%s", b->last_service_date);
    } while (!isValidDate(b->last_service_date));

    predictMaintenance(b);
    fleet_size++;

    printf(GREEN "\n✔ Bus added successfully.\n" RESET);
}

void sortFleetByDue() {
    for (int i = 0; i < fleet_size - 1; i++) {
        for (int j = i + 1; j < fleet_size; j++) {
            if (fleet[i].next_due_mileage > fleet[j].next_due_mileage) {
                Bus temp = fleet[i];
                fleet[i] = fleet[j];
                fleet[j] = temp;
            }
        }
    }
}

void displayFleet() {
    if (fleet_size == 0) {
        printf(YELLOW "\nNo buses to display.\n" RESET);
        return;
    }

    sortFleetByDue();

    printf("\n======================= FLEET DETAILS =======================\n");
    for (int i = 0; i < fleet_size; i++) {
        Bus *b = &fleet[i];

        printf("\n--------------------------------------------------------------\n");
        printf("Bus No: %d\n", b->bus_no);
        printf("Last Service Date     : %s\n", b->last_service_date);
        printf("Mileage at Service    : %d km\n", b->last_service_mileage);
        printf("Current Mileage       : %d km\n", b->current_mileage);
        printf("Next Due Mileage      : %d km\n", b->next_due_mileage);
        printf("Days Since Service    : %d days\n", b->days_since_service);
        printf("Due in (Time)         : %d days\n", b->due_in_days);

        if (b->overdue_flag)
            printf("STATUS                : " RED "⚠ OVERDUE\n" RESET);
        else
            printf("STATUS                : " GREEN "OK\n" RESET);
    }
}

void updateMileage() {
    printf("\nEnter Bus Number to Update: ");
    int bus = askInt();

    for (int i = 0; i < fleet_size; i++) {
        if (fleet[i].bus_no == bus) {
            printf("Enter New Mileage: ");
            int m = askInt();
            if (m < fleet[i].current_mileage) {
                printf(RED "Mileage cannot decrease.\n" RESET);
                return;
            }
            fleet[i].current_mileage = m;
            predictMaintenance(&fleet[i]);
            printf(GREEN "✔ Mileage updated.\n" RESET);
            return;
        }
    }

    printf(RED "Bus not found.\n" RESET);
}

void searchBus() {
    printf("\nEnter Bus Number to Search: ");
    int bus = askInt();
    for (int i = 0; i < fleet_size; i++) {
        if (fleet[i].bus_no == bus) {
            printf(GREEN "Bus Found!\n" RESET);
            printf("Mileage: %d | Due at: %d\n",
                fleet[i].current_mileage,
                fleet[i].next_due_mileage);
            return;
        }
    }
    printf(RED "Bus not found.\n" RESET);
}

void logOverdue(Bus *b) {
    FILE *f = fopen(LOG_FILE, "a");
    fprintf(f, "Bus %d overdue on mileage=%d days_since=%d\n",
            b->bus_no, b->current_mileage, b->days_since_service);
    fclose(f);
}

void checkOverdue() {
    printf("\n=========== OVERDUE BUSES ===========\n");
    int found = 0;

    for (int i = 0; i < fleet_size; i++) {
        if (fleet[i].overdue_flag) {
            printf(RED "Bus %d is OVERDUE.\n" RESET, fleet[i].bus_no);
            logOverdue(&fleet[i]);
            found = 1;
        }
    }

    if (!found) printf(GREEN "No overdue buses.\n" RESET);
}

void saveToFile() {
    FILE *f = fopen(DATA_FILE, "w");
    for (int i = 0; i < fleet_size; i++) {
        Bus *b = &fleet[i];
        fprintf(f, "%d,%d,%d,%s,%d,%d,%d,%d\n",
                b->bus_no, b->last_service_mileage, b->current_mileage,
                b->last_service_date, b->next_due_mileage,
                b->days_since_service, b->due_in_days, b->overdue_flag);
    }
    fclose(f);
    printf(GREEN "\n✔ Data saved to file.\n" RESET);
}

void loadFromFile() {
    FILE *f = fopen(DATA_FILE, "r");
    if (!f) {
        printf(YELLOW "No data file exists.\n" RESET);
        return;
    }

    fleet_size = 0;
    fleet = realloc(fleet, sizeof(Bus));

    while (!feof(f)) {
        Bus b;
        if (fscanf(f, "%d,%d,%d,%[^,],%d,%d,%d,%d\n",
                   &b.bus_no, &b.last_service_mileage, &b.current_mileage,
                   b.last_service_date, &b.next_due_mileage,
                   &b.days_since_service, &b.due_in_days, &b.overdue_flag) == 8) {

            fleet = realloc(fleet, (fleet_size + 1) * sizeof(Bus));
            fleet[fleet_size++] = b;
        }
    }
    fclose(f);

    printf(GREEN "✔ Data loaded successfully.\n" RESET);
}

int main() {
    header();
    loadFromFile();

    int ch;

    while (1) {
        printf(BLUE "\n==================== MENU ====================\n" RESET);
        printf("1. Add Bus\n");
        printf("2. Display Fleet\n");
        printf("3. Update Mileage\n");
        printf("4. Check Overdue\n");
        printf("5. Search Bus\n");
        printf("6. Save to File\n");
        printf("7. Exit\n");
        printf("Enter Choice: ");

        ch = askInt();

        switch (ch) {
            case 1: addBus(); break;
            case 2: displayFleet(); break;
            case 3: updateMileage(); break;
            case 4: checkOverdue(); break;
            case 5: searchBus(); break;
            case 6: saveToFile(); break;
            case 7:
                saveToFile();
                printf(GREEN "Exiting...\n" RESET);
                return 0;
            default:
                printf(RED "Invalid Option!\n" RESET);
        }
    }

    return 0;
}