/**********************************************************************
 * PROJECT:   Bus Fleet Maintenance Tracker (Project 69) - FleetGuardian
 * LANGUAGE:  C
 *
 * DESCRIPTION:
 *   Console-based maintenance tracker for a bus fleet. Maintains a
 *   dynamic list of buses, predicts next maintenance, and generates
 *   alerts based on mileage and optional date intervals.
 *
 *   KEY FEATURES:
 *     - Dynamic fleet storage using malloc/realloc (pointers)
 *     - Arrays of structures to hold per-bus data
 *     - Robust input validation (re-prompts on invalid input)
 *     - Full driver name (with spaces)
 *     - Reference date for maintenance checking
 *     - Status bands: OK, DUE SOON, OVERDUE
 *     - Computed health score (0–100) per bus
 *     - Search, edit (by position), delete, and summary views
 *     - Save/load fleet from text file (bus_data.txt)
 *     - Export maintenance report to a CSV file
 *
 *  Developed By: Shayan Shome
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ---------- ANSI colors ---------- */

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_BOLD    "\033[1m"

/* ---------- Constants & filenames ---------- */

#define DUE_SOON_KM   500
#define DATA_FILE     "bus_data.txt"
#define REPORT_FILE   "fleet_report.csv"

/* ---------- Status & Data Structures ---------- */

typedef enum {
    STATUS_OK = 0,
    STATUS_DUE_SOON = 1,
    STATUS_OVERDUE = 2
} Status;

typedef struct {
    int day, month, year;
} Date;

typedef struct {
    char  bus_code[20];
    char  driver_name[50];

    int   bus_no;
    Date  last_service;
    Date  next_due;

    float current_mileage;
    float last_service_mileage;
    float service_interval_km;
    int   service_interval_days;

    int   service_history_count;
    Status status;

    float km_left;
    int   health_score;
    float avg_daily_km;
    float fuel_efficiency;
} Bus;

/* ---------- Banner / UI helpers ---------- */

void print_banner(void) {
    printf(COLOR_CYAN COLOR_BOLD
           "\n=============================================\n"
           "               FleetGuardian\n"
           "   Intelligent Bus Fleet Maintenance Tracker\n"
           "=============================================\n"
           COLOR_RESET);
}

/* ---------- String helpers (case-insensitive) ---------- */

int str_ieq(const char *a, const char *b) {
    /* return 1 if equal ignoring case, else 0 */
    unsigned char ca, cb;
    while (*a && *b) {
        ca = (unsigned char)*a;
        cb = (unsigned char)*b;
        if (tolower(ca) != tolower(cb)) {
            return 0;
        }
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

void to_upper_str(char *s) {
    while (*s) {
        *s = (char)toupper((unsigned char)*s);
        s++;
    }
}

/* ---------- Safe input helpers ---------- */

int read_line_stdin(char *buf, int size) {
    if (!fgets(buf, size, stdin)) {
        buf[0] = '\0';
        return 0;
    }
    size_t len = strlen(buf);
    while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) {
        buf[--len] = '\0';
    }
    return 1;
}

static int is_all_digits(const char *s) {
    int has_digit = 0;
    while (*s) {
        if (!isdigit((unsigned char)*s) && !isspace((unsigned char)*s))
            return 0;
        if (isdigit((unsigned char)*s))
            has_digit = 1;
        s++;
    }
    return has_digit;
}

int read_int_strict(const char *prompt, int min, int max) {
    char buf[128];
    char *endptr;
    long val;

    while (1) {
        printf("%s", prompt);
        if (!read_line_stdin(buf, sizeof buf))
            continue;
        if (buf[0] == '\0') {
            printf(COLOR_RED "Input cannot be empty.\n" COLOR_RESET);
            continue;
        }
        val = strtol(buf, &endptr, 10);
        while (*endptr && isspace((unsigned char)*endptr)) endptr++;
        if (*endptr != '\0') {
            printf(COLOR_RED "Invalid input. Please enter digits only.\n"
                   COLOR_RESET);
            continue;
        }
        if (val < min || val > max) {
            printf(COLOR_YELLOW
                   "Please enter a value between %d and %d.\n"
                   COLOR_RESET, min, max);
            continue;
        }
        return (int)val;
    }
}

float read_float_strict(const char *prompt, float min, float max) {
    char buf[128];
    char *endptr;
    float val;

    while (1) {
        printf("%s", prompt);
        if (!read_line_stdin(buf, sizeof buf))
            continue;
        if (buf[0] == '\0') {
            printf(COLOR_RED "Input cannot be empty.\n" COLOR_RESET);
            continue;
        }
        val = strtof(buf, &endptr);
        while (*endptr && isspace((unsigned char)*endptr)) endptr++;
        if (*endptr != '\0') {
            printf(COLOR_RED "Invalid input. Please enter a numeric value.\n"
                   COLOR_RESET);
            continue;
        }
        if (val < min || val > max) {
            printf(COLOR_YELLOW
                   "Please enter a value between %.1f and %.1f.\n"
                   COLOR_RESET, min, max);
            continue;
        }
        return val;
    }
}

void read_driver_name(char *dest, int size) {
    char buf[128];
    while (1) {
        printf("Enter driver name (full name): ");
        if (!read_line_stdin(buf, sizeof buf))
            continue;
        if (buf[0] == '\0') {
            printf(COLOR_RED "Name cannot be empty.\n" COLOR_RESET);
            continue;
        }
        if (is_all_digits(buf)) {
            printf(COLOR_RED
                   "Name cannot be only numbers. Please enter a proper name.\n"
                   COLOR_RESET);
            continue;
        }
        strncpy(dest, buf, size - 1);
        dest[size - 1] = '\0';
        return;
    }
}

/* ---------- Date helpers (simplified) ---------- */

int is_valid_date(Date d) {
    if (d.year <= 0 || d.month < 1 || d.month > 12 || d.day < 1 || d.day > 31)
        return 0;
    return 1;
}

Date read_date(const char *prompt) {
    Date d;
    char buf[128];
    while (1) {
        printf("%s", prompt);
        if (!read_line_stdin(buf, sizeof buf))
            continue;
        if (sscanf(buf, "%d/%d/%d", &d.day, &d.month, &d.year) == 3 &&
            is_valid_date(d)) {
            return d;
        }
        printf(COLOR_RED
               "Invalid date. Use format dd/mm/yyyy with valid values.\n"
               COLOR_RESET);
    }
}

int date_to_days(Date d) {
    return d.year * 365 + d.month * 30 + d.day;
}

Date days_to_date(int total) {
    Date d;
    d.year = total / 365;
    int rem = total % 365;
    d.month = rem / 30;
    if (d.month == 0) d.month = 1;
    d.day = rem % 30;
    if (d.day == 0) d.day = 1;
    return d;
}

Date add_days(Date d, int days) {
    int total = date_to_days(d) + days;
    return days_to_date(total);
}

void print_date(Date d) {
    printf("%02d-%02d-%04d", d.day, d.month, d.year);
}

/* ---------- Maintenance logic ---------- */

void update_maintenance_status(Bus *b, Date today) {
    float due_mileage = b->last_service_mileage + b->service_interval_km;
    b->km_left = due_mileage - b->current_mileage;

    int mileage_overdue = (b->current_mileage >= due_mileage);
    int mileage_due_soon = (!mileage_overdue && b->km_left <= DUE_SOON_KM);

    int date_overdue = 0;
    if (b->service_interval_days > 0 &&
        is_valid_date(b->last_service) &&
        is_valid_date(today)) {
        int days_since = date_to_days(today) - date_to_days(b->last_service);
        if (days_since >= b->service_interval_days) {
            date_overdue = 1;
        }
        b->next_due = add_days(b->last_service, b->service_interval_days);
    } else {
        b->next_due.day = b->next_due.month = b->next_due.year = 0;
    }

    if (mileage_overdue || date_overdue) {
        b->status = STATUS_OVERDUE;
    } else if (mileage_due_soon) {
        b->status = STATUS_DUE_SOON;
    } else {
        b->status = STATUS_OK;
    }

    float used = b->current_mileage - b->last_service_mileage;
    if (b->service_interval_km > 0.0f) {
        float ratio = used / b->service_interval_km;
        if (ratio < 0.0f) ratio = 0.0f;
        if (ratio > 1.5f) ratio = 1.5f;
        b->health_score = (int)((1.5f - ratio) / 1.5f * 100.0f);
        if (b->health_score < 0)   b->health_score = 0;
        if (b->health_score > 100) b->health_score = 100;
    } else {
        b->health_score = 50;
    }
}

/* ---------- Search, status & uniqueness helpers ---------- */

int find_bus_index(Bus *fleet, int count, int bus_no) {
    for (int i = 0; i < count; i++) {
        if (fleet[i].bus_no == bus_no) return i;
    }
    return -1;
}

const char* status_label(Status s) {
    switch (s) {
        case STATUS_OK:       return "OK";
        case STATUS_DUE_SOON: return "DUE SOON";
        case STATUS_OVERDUE:  return "OVERDUE";
        default:              return "UNKNOWN";
    }
}

const char* status_color(Status s) {
    switch (s) {
        case STATUS_OK:       return COLOR_GREEN;
        case STATUS_DUE_SOON: return COLOR_YELLOW;
        case STATUS_OVERDUE:  return COLOR_RED;
        default:              return COLOR_RESET;
    }
}

/* exclude_index = -1 when adding; otherwise skip that index while editing */
int bus_code_exists(Bus *fleet, int count, const char *code, int exclude_index) {
    for (int i = 0; i < count; i++) {
        if (i == exclude_index) continue;
        if (str_ieq(fleet[i].bus_code, code)) {
            return 1;
        }
    }
    return 0;
}

int bus_no_exists(Bus *fleet, int count, int bus_no, int exclude_index) {
    for (int i = 0; i < count; i++) {
        if (i == exclude_index) continue;
        if (fleet[i].bus_no == bus_no) {
            return 1;
        }
    }
    return 0;
}

/* ---------- File I/O: save / load ---------- */

void save_fleet_to_file(Bus *fleet, int count, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        printf(COLOR_RED "Error opening file for writing.\n" COLOR_RESET);
        return;
    }

    fprintf(fp, "%d\n", count);
    for (int i = 0; i < count; i++) {
        Bus *b = &fleet[i];
        fprintf(fp,
                "%s|%s|%d|"
                "%d|%d|%d|"
                "%d|%d|%d|"
                "%.2f|%.2f|%.2f|"
                "%d|%d|%d|"
                "%.2f|%d|%.2f|%.2f\n",
                b->bus_code,
                b->driver_name,
                b->bus_no,
                b->last_service.day,
                b->last_service.month,
                b->last_service.year,
                b->next_due.day,
                b->next_due.month,
                b->next_due.year,
                b->current_mileage,
                b->last_service_mileage,
                b->service_interval_km,
                b->service_interval_days,
                b->service_history_count,
                b->status,
                b->km_left,
                b->health_score,
                b->avg_daily_km,
                b->fuel_efficiency);
    }

    fclose(fp);
    printf(COLOR_GREEN "Fleet saved to %s\n" COLOR_RESET, filename);
}

void load_fleet_from_file(Bus **fleet_ptr, int *count, int *capacity,
                          const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        *count = 0;
        return;
    }

    int n;
    if (fscanf(fp, "%d\n", &n) != 1 || n <= 0) {
        printf(COLOR_YELLOW "Data file empty or invalid.\n" COLOR_RESET);
        fclose(fp);
        *count = 0;
        return;
    }

    if (n > *capacity) {
        Bus *tmp = realloc(*fleet_ptr, n * sizeof(Bus));
        if (!tmp) {
            printf(COLOR_RED
                   "Memory allocation failed while loading file.\n"
                   COLOR_RESET);
            fclose(fp);
            *count = 0;
            return;
        }
        *fleet_ptr = tmp;
        *capacity = n;
    }

    for (int i = 0; i < n; i++) {
        Bus *b = &((*fleet_ptr)[i]);
        int status_int;
        if (fscanf(fp,
                   "%19[^|]|%49[^|]|%d|"
                   "%d|%d|%d|"
                   "%d|%d|%d|"
                   "%f|%f|%f|"
                   "%d|%d|%d|"
                   "%f|%d|%f|%f\n",
                   b->bus_code,
                   b->driver_name,
                   &b->bus_no,
                   &b->last_service.day,
                   &b->last_service.month,
                   &b->last_service.year,
                   &b->next_due.day,
                   &b->next_due.month,
                   &b->next_due.year,
                   &b->current_mileage,
                   &b->last_service_mileage,
                   &b->service_interval_km,
                   &b->service_interval_days,
                   &b->service_history_count,
                   &status_int,
                   &b->km_left,
                   &b->health_score,
                   &b->avg_daily_km,
                   &b->fuel_efficiency) != 19) {
            printf(COLOR_YELLOW "Warning: corrupted line in data file.\n" COLOR_RESET);
        }
        b->status = (Status)status_int;
    }

    fclose(fp);
    *count = n;
    printf(COLOR_GREEN "Loaded %d buses from %s\n" COLOR_RESET, *count, filename);
}

/* ---------- Display / Search / Reports ---------- */

void display_one_bus(Bus *b) {
    const char *col = status_color(b->status);
    printf("%sBus %d [%s] (%s)%s\n",
           col, b->bus_no, b->bus_code, status_label(b->status), COLOR_RESET);
    printf("  Driver name       : %s\n", b->driver_name);
    printf("  Last service date : ");
    print_date(b->last_service);
    printf("\n");
    if (b->next_due.year > 0) {
        printf("  Next due date     : ");
        print_date(b->next_due);
        printf("\n");
    }
    printf("  Last service km   : %.1f\n", b->last_service_mileage);
    printf("  Current km        : %.1f\n", b->current_mileage);
    printf("  Interval          : %.1f km, %d days\n",
           b->service_interval_km, b->service_interval_days);
    printf("  Km left           : %.1f\n", b->km_left);
    printf("  Avg daily km      : %.1f\n", b->avg_daily_km);
    printf("  Fuel efficiency   : %.1f km/l\n", b->fuel_efficiency);
    printf("  Health score      : %d/100\n", b->health_score);
    printf("  Service history   : %d\n", b->service_history_count);
}

void display_all_buses(Bus *fleet, int count) {
    if (count == 0) {
        printf(COLOR_YELLOW "No buses in fleet.\n" COLOR_RESET);
        return;
    }

    printf(COLOR_BOLD
           "\n================ Fleet Summary (All Buses) ================\n"
           COLOR_RESET);
    printf("Total buses: %d\n\n", count);

    printf("Bus  | Code      | Driver        | Last Service | Next Due   | CurrKm     | KmLeft    | Health   | Status   \n");
    printf("-----+-----------+---------------+--------------+------------+------------+-----------+----------+---------\n");

    for (int i = 0; i < count; i++) {
        Bus *b = &fleet[i];
        const char *col = status_color(b->status);
        char last_buf[16];
        char next_buf[16];

        snprintf(last_buf, sizeof last_buf, "%02d-%02d-%04d",
                 b->last_service.day, b->last_service.month, b->last_service.year);

        if (b->next_due.year > 0) {
            snprintf(next_buf, sizeof next_buf, "%02d-%02d-%04d",
                     b->next_due.day, b->next_due.month, b->next_due.year);
        } else {
            strcpy(next_buf, "-");
        }

        printf("%-4d | %-9.9s | %-13.13s | %-12s | %-10s | %10.1f | %9.1f | %8d | %s%-9s%s\n",
               b->bus_no,
               b->bus_code,
               b->driver_name,
               last_buf,
               next_buf,
               b->current_mileage,
               b->km_left,
               b->health_score,
               col, status_label(b->status), COLOR_RESET);
    }

    printf("\n");
}

void show_due_soon_or_overdue(Bus *fleet, int count) {
    int found = 0;
    printf(COLOR_BOLD "\n=== Buses Due Soon / Overdue ===\n" COLOR_RESET);
    for (int i = 0; i < count; i++) {
        if (fleet[i].status == STATUS_DUE_SOON ||
            fleet[i].status == STATUS_OVERDUE) {
            display_one_bus(&fleet[i]);
            printf("\n");
            found = 1;
        }
    }
    if (!found) {
        printf(COLOR_GREEN
               "No maintenance due right now or in the next few days.\n"
               COLOR_RESET);
    }
}

void search_bus(Bus *fleet, int count) {
    int bus_no = read_int_strict("Enter bus number to search: ", 1, 9999999);

    int idx = find_bus_index(fleet, count, bus_no);
    if (idx == -1) {
        printf(COLOR_RED "Bus not found.\n" COLOR_RESET);
        return;
    }
    display_one_bus(&fleet[idx]);
}

/* ---------- Edit by position ---------- */

int choose_bus_position(Bus *fleet, int count) {
    if (count == 0) {
        printf(COLOR_YELLOW "No buses available to select.\n" COLOR_RESET);
        return -1;
    }

    printf("\nAvailable buses (positions):\n");
    printf("Pos | BusNo | Code        | Driver\n");
    printf("----+-------+-------------+----------------\n");
    for (int i = 0; i < count; i++) {
        printf("%-3d | %-5d | %-11.11s | %-16.16s\n",
               i + 1,
               fleet[i].bus_no,
               fleet[i].bus_code,
               fleet[i].driver_name);
    }

    int pos = read_int_strict("\nEnter position: ", 1, count);
    return pos - 1;
}

void edit_bus_by_position(Bus *fleet, int count) {
    int idx = choose_bus_position(fleet, count);
    if (idx < 0) return;

    Bus *b = &fleet[idx];
    printf(COLOR_CYAN "Editing position %d (Bus %d, %s)\n"
           COLOR_RESET, idx + 1, b->bus_no, b->bus_code);

    char tmp[64];

    /* Unique bus_code (allow keeping same, case-insensitive) */
    while (1) {
        printf("Enter new bus code (leave empty to keep '%s'): ",
               b->bus_code);
        if (!read_line_stdin(tmp, sizeof tmp))
            continue;
        if (tmp[0] == '\0') {
            /* keep old */
            break;
        }
        to_upper_str(tmp);
        if (bus_code_exists(fleet, count, tmp, idx)) {
            printf(COLOR_RED
                   "This bus code already exists (case-insensitive). Please enter a different code.\n"
                   COLOR_RESET);
            continue;
        }
        strncpy(b->bus_code, tmp, sizeof b->bus_code - 1);
        b->bus_code[sizeof b->bus_code - 1] = '\0';
        break;
    }

    /* Unique bus_no (allow keeping same) */
    while (1) {
        int new_no = read_int_strict(
            "Enter new numeric bus number (or same as before): ",
            1, 9999999);
        if (bus_no_exists(fleet, count, new_no, idx)) {
            printf(COLOR_RED
                   "This bus number already exists. Please enter a different number.\n"
                   COLOR_RESET);
            continue;
        }
        b->bus_no = new_no;
        break;
    }

    read_driver_name(b->driver_name, sizeof b->driver_name);

    b->last_service = read_date("Enter new last service date (dd/mm/yyyy): ");

    b->last_service_mileage =
        read_float_strict("Enter new last service mileage (km): ",
                          0.0f, 100000.0f);

    b->current_mileage =
        read_float_strict("Enter new current mileage (km): ",
                          0.0f, 100000.0f);

    b->service_interval_km =
        read_float_strict("Enter new service interval (km): ",
                          1.0f, 100000.0f);

    b->service_interval_days =
        read_int_strict("Enter new service interval in days (0 if not used): ",
                        0, 5000);

    b->avg_daily_km =
        read_float_strict("Enter new average daily km: ",
                          0.0f, 100000.0f);

    b->fuel_efficiency =
        read_float_strict("Enter new fuel efficiency (km/l): ",
                          0.0f, 100.0f);

    b->service_history_count =
        read_int_strict("Enter new service history count: ",
                        0, 1500);

    printf(COLOR_GREEN "Bus at position %d updated.\n" COLOR_RESET, idx + 1);
}

/* ---------- Add / update / delete ---------- */

void add_bus(Bus **fleet_ptr, int *count, int *capacity) {
    if (*count >= *capacity) {
        int new_cap = (*capacity == 0) ? 4 : (*capacity * 2);
        Bus *tmp = realloc(*fleet_ptr, new_cap * sizeof(Bus));
        if (!tmp) {
            printf(COLOR_RED "Memory allocation failed while adding bus.\n"
                   COLOR_RESET);
            return;
        }
        *fleet_ptr = tmp;
        *capacity = new_cap;
    }

    Bus *b = &((*fleet_ptr)[*count]);
    char tmp[64];

    /* Unique bus_code (case-insensitive, normalised to upper-case) */
    while (1) {
        printf("Enter bus code (e.g. CHD-101A): ");
        if (!read_line_stdin(tmp, sizeof tmp) || tmp[0] == '\0') {
            printf(COLOR_RED "Code cannot be empty.\n" COLOR_RESET);
            continue;
        }
        to_upper_str(tmp);
        if (bus_code_exists(*fleet_ptr, *count, tmp, -1)) {
            printf(COLOR_RED
                   "This bus code already exists (case-insensitive). Please enter a different code.\n"
                   COLOR_RESET);
            continue;
        }
        strncpy(b->bus_code, tmp, sizeof b->bus_code - 1);
        b->bus_code[sizeof b->bus_code - 1] = '\0';
        break;
    }

    /* Unique bus_no */
    while (1) {
        int no = read_int_strict("Enter numeric bus number: ", 1, 9999999);
        if (bus_no_exists(*fleet_ptr, *count, no, -1)) {
            printf(COLOR_RED
                   "This bus number already exists. Please enter a different number.\n"
                   COLOR_RESET);
            continue;
        }
        b->bus_no = no;
        break;
    }

    read_driver_name(b->driver_name, sizeof b->driver_name);

    b->last_service = read_date("Enter last service date (dd/mm/yyyy): ");

    b->last_service_mileage =
        read_float_strict("Enter last service mileage (km): ",
                          0.0f, 100.0f);

    b->current_mileage =
        read_float_strict("Enter current mileage (km): ",
                          0.0f, 100.0f);

    b->service_interval_km =
        read_float_strict("Enter service interval (km), e.g. 10000: ",
                          1.0f, 100000.0f);

    b->service_interval_days =
        read_int_strict("Enter service interval in days (0 if not used): ",
                        0, 5000);

    b->avg_daily_km =
        read_float_strict("Enter average daily km: ",
                          0.0f, 100000.0f);

    b->fuel_efficiency =
        read_float_strict("Enter fuel efficiency (km/l): ",
                          0.0f, 200.0f);

    b->service_history_count =
        read_int_strict("Enter service history count: ",
                        0, 1500);

    b->next_due.day = b->next_due.month = b->next_due.year = 0;
    b->km_left = 0.0f;
    b->status = STATUS_OK;
    b->health_score = 100;

    (*count)++;
    printf(COLOR_GREEN "Bus added. Total buses: %d\n" COLOR_RESET, *count);
}

void update_mileage(Bus *fleet, int count) {
    int bus_no = read_int_strict("Enter bus number to update mileage: ",
                                 1, 9999999);

    int idx = find_bus_index(fleet, count, bus_no);
    if (idx == -1) {
        printf(COLOR_RED "Bus not found.\n" COLOR_RESET);
        return;
    }

    Bus *b = &fleet[idx];
    printf("Current mileage for bus %d: %.1f km\n",
           b->bus_no, b->current_mileage);
    b->current_mileage =
        read_float_strict("Enter new current mileage (km): ",
                          0.0f, 100000000.0f);
    printf(COLOR_GREEN "Mileage updated.\n" COLOR_RESET);
}

void delete_bus(Bus *fleet, int *count) {
    int bus_no = read_int_strict("Enter bus number to delete: ",
                                 1, 9999999);

    int idx = find_bus_index(fleet, *count, bus_no);
    if (idx == -1) {
        printf(COLOR_RED "Bus not found.\n" COLOR_RESET);
        return;
    }

    for (int i = idx; i < *count - 1; i++) {
        fleet[i] = fleet[i + 1];
    }
    (*count)--;
    printf(COLOR_YELLOW "Bus deleted. Remaining: %d\n" COLOR_RESET, *count);
}

/* ---------- Quick summary after entering reference date ---------- */

void summarize_maintenance(Bus *fleet, int count, Date today) {
    if (count == 0) {
        printf(COLOR_YELLOW
               "No buses in fleet yet. Add bus data to check maintenance.\n"
               COLOR_RESET);
        return;
    }

    int overdue = 0;
    int due_soon = 0;

    for (int i = 0; i < count; i++) {
        update_maintenance_status(&fleet[i], today);
        if (fleet[i].status == STATUS_OVERDUE) {
            overdue++;
        } else if (fleet[i].status == STATUS_DUE_SOON) {
            due_soon++;
        }
    }

    if (overdue == 0 && due_soon == 0) {
        printf(COLOR_GREEN
               "No maintenance due right now, or upcoming in the next few days.\n"
               COLOR_RESET);
        return;
    }

    if (overdue > 0) {
        printf(COLOR_RED
               "\nThese buses NEED maintenance on or before the chosen date:\n"
               COLOR_RESET);
        for (int i = 0; i < count; i++) {
            if (fleet[i].status == STATUS_OVERDUE) {
                printf("  - Bus %d [%s] (driver: %s)\n",
                       fleet[i].bus_no,
                       fleet[i].bus_code,
                       fleet[i].driver_name);
            }
        }
    }

    if (due_soon > 0) {
        printf(COLOR_YELLOW
               "\nThese buses will need maintenance SOON (within %d km):\n"
               COLOR_RESET, DUE_SOON_KM);
        for (int i = 0; i < count; i++) {
            if (fleet[i].status == STATUS_DUE_SOON) {
                printf("  - Bus %d [%s] (driver: %s), km left: %.1f\n",
                       fleet[i].bus_no,
                       fleet[i].bus_code,
                       fleet[i].driver_name,
                       fleet[i].km_left);
            }
        }
    }

    printf("\n");
}

/* ---------- CSV export ---------- */

void export_report(Bus *fleet, int count, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        printf(COLOR_RED "Could not open report file.\n" COLOR_RESET);
        return;
    }

    fprintf(fp,
            "BusNo,BusCode,DriverName,LastServiceDate,NextDueDate,"
            "CurrentKm,KmLeft,HealthScore,Status,ServiceHistoryCount\n");

    for (int i = 0; i < count; i++) {
        Bus *b = &fleet[i];
        char last_buf[16];
        char next_buf[16];

        snprintf(last_buf, sizeof last_buf, "%02d-%02d-%04d",
                 b->last_service.day, b->last_service.month, b->last_service.year);
        if (b->next_due.year > 0)
            snprintf(next_buf, sizeof next_buf, "%02d-%02d-%04d",
                     b->next_due.day, b->next_due.month, b->next_due.year);
        else
            next_buf[0] = '\0';

        fprintf(fp,
                "%d,\"%s\",\"%s\",\"%s\",\"%s\",%.1f,%.1f,%d,\"%s\",%d\n",
                b->bus_no,
                b->bus_code,
                b->driver_name,
                last_buf,
                next_buf,
                b->current_mileage,
                b->km_left,
                b->health_score,
                status_label(b->status),
                b->service_history_count);
    }

    fclose(fp);
    printf(COLOR_GREEN "CSV report exported to %s\n" COLOR_RESET, filename);
}

/* ---------- Main menu ---------- */

int main(void) {
    Bus *fleet = NULL;
    int count = 0;
    int capacity = 0;
    Date today;

    print_banner();

    load_fleet_from_file(&fleet, &count, &capacity, DATA_FILE);

    today = read_date("Enter reference date for maintenance check (dd/mm/yyyy): ");

    summarize_maintenance(fleet, count, today);

    int choice;
    do {
        for (int i = 0; i < count; i++) {
            update_maintenance_status(&fleet[i], today);
        }

        printf(COLOR_BOLD "-------------- Main Menu --------------\n" COLOR_RESET);
        printf("Current reference date: ");
        print_date(today);
        printf("\n");
        printf("---------------------------------------\n");
        printf("1. Change reference date (dd/mm/yyyy)\n");
        printf("2. Add new bus\n");
        printf("3. Edit existing bus details\n");
        printf("4. Update mileage\n");
        printf("5. Delete bus\n");
        printf("6. Search by bus number\n");
        printf("7. View all buses (all data)\n");
        printf("8. Show buses due soon / overdue\n");
        printf("9. Export maintenance report (CSV)\n");
        printf("10. Save & exit\n");
        printf("---------------------------------------\n");

        choice = read_int_strict("Enter choice: ", 1, 10);

        switch (choice) {
            case 1:
                today = read_date("Enter new reference date (dd/mm/yyyy): ");
                printf(COLOR_GREEN "Reference date updated to: " COLOR_RESET);
                print_date(today);
                printf("\n");
                summarize_maintenance(fleet, count, today);
                break;
            case 2: add_bus(&fleet, &count, &capacity); break;
            case 3: edit_bus_by_position(fleet, count); break;
            case 4: update_mileage(fleet, count); break;
            case 5: delete_bus(fleet, &count); break;
            case 6: search_bus(fleet, count); break;
            case 7: display_all_buses(fleet, count); break;
            case 8: show_due_soon_or_overdue(fleet, count); break;
            case 9: export_report(fleet, count, REPORT_FILE); break;
            case 10:
                save_fleet_to_file(fleet, count, DATA_FILE);
                printf(COLOR_CYAN "Goodbye. Data saved.\n" COLOR_RESET);
                break;
        }
    } while (choice != 10);

    free(fleet);
    return 0;
}
