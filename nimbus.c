#include <stdio.h>
#include <stdlib.h>

struct Bus {
    int bus_no;
    int last_service;  
    int next_due;      
    int mileage;       
};

void addBus(struct Bus *b) {
    printf("\nEnter Bus Number: ");
    scanf("%d", &b->bus_no);
    
    printf("Enter Last Service Mileage: ");
    scanf("%d", &b->last_service);

    printf("Enter Current Mileage: ");
    scanf("%d", &b->mileage);

    b->next_due = b->last_service + 5000;
}

void display(struct Bus b) {
    printf("\nBus No: %d", b.bus_no);
    printf("\nLast Service: %d km", b.last_service);
    printf("\nCurrent Mileage: %d km", b.mileage);
    printf("\nNext Service Due: %d km\n", b.next_due);
}

void checkOverdue(struct Bus b) {
    if (b.mileage >= b.next_due) {
        printf("⚠ Bus %d is OVERDUE for maintenance!\n", b.bus_no);
    } else {
        printf("✔ Bus %d is within service limit.\n", b.bus_no);
    }
}

int main() {
    int n, i;
    printf("Enter number of buses: ");
    scanf("%d", &n);

    struct Bus *fleet = (struct Bus *)malloc(n * sizeof(struct Bus));

    for (i = 0; i < n; i++) {
        printf("\n--- Enter Details for Bus %d ---\n", i + 1);
        addBus(&fleet[i]);
    }

    printf("\n===== BUS DETAILS =====\n");
    for (i = 0; i < n; i++) {
        display(fleet[i]);
    }

    printf("\n===== OVERDUE CHECK =====\n");
    for (i = 0; i < n; i++) {
        checkOverdue(fleet[i]);
    }

    free(fleet);
    return 0;
}
