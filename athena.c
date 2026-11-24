/*
 * Athena Registration System (console, C language)
 * - College registration
 * - Student participation list
 * - Stops (with order and pickup time) and transport requirement
 * - Bus allocation + summary (students by stop, bus no., timings)
 * Data is saved in "athena.dat" so it stays after you close the program.
 *
 * Compile: gcc athena.c -o athena
 * Run: ./athena (Windows: athena.exe)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COLLEGES 50
#define MAX_STUDENTS 500
#define MAX_STOPS 20
#define LEN 50
#define DATA_FILE "athena.dat"

typedef struct {
    int id;
    char name[LEN];
    char coordinator[LEN];
    char phone[16];
} College;

typedef struct {
    int id;
    int collegeId;
    char name[LEN];
    char roll[20];
    char event[LEN];
    int needsTransport;
    int stopId;
    int busNo;
} Student;

typedef struct {
    int id;
    char name[LEN];
    int order;
    char time[10];
} Stop;

College colleges[MAX_COLLEGES];
Student students[MAX_STUDENTS];
Stop stops[MAX_STOPS];
int nColleges = 0, nStudents = 0, nStops = 0;
int busesUsed = 0, busCapacity = 0;

void readLine(const char *prompt, char *buf, int size) {
    printf("%s", prompt);
    if (fgets(buf, size, stdin) == NULL) { buf[0] = '\0'; return; }
    buf[strcspn(buf, "\n")] = '\0';
}

int readInt(const char *prompt) {
    char tmp[32];
    int v;
    while (1) {
        readLine(prompt, tmp, sizeof tmp);
        if (sscanf(tmp, "%d", &v) == 1) return v;
        printf("Please enter a number.\n");
    }
}

void saveData(void) {
    FILE *f = fopen(DATA_FILE, "wb");
    if (!f) { printf("Could not save data!\n"); return; }
    fwrite(&nColleges, sizeof nColleges, 1, f);
    fwrite(colleges, sizeof(College), nColleges, f);
    fwrite(&nStudents, sizeof nStudents, 1, f);
    fwrite(students, sizeof(Student), nStudents, f);
    fwrite(&nStops, sizeof nStops, 1, f);
    fwrite(stops, sizeof(Stop), nStops, f);
    fwrite(&busesUsed, sizeof busesUsed, 1, f);
    fwrite(&busCapacity, sizeof busCapacity, 1, f);
    fclose(f);
}

void loadData(void) {
    FILE *f = fopen(DATA_FILE, "rb");
    if (!f) return;
    if (fread(&nColleges, sizeof nColleges, 1, f) != 1) goto bad;
    if (nColleges < 0 || nColleges > MAX_COLLEGES) goto bad;
    if (fread(colleges, sizeof(College), nColleges, f) != (size_t)nColleges) goto bad;
    if (fread(&nStudents, sizeof nStudents, 1, f) != 1) goto bad;
    if (nStudents < 0 || nStudents > MAX_STUDENTS) goto bad;
    if (fread(students, sizeof(Student), nStudents, f) != (size_t)nStudents) goto bad;
    if (fread(&nStops, sizeof nStops, 1, f) != 1) goto bad;
    if (nStops < 0 || nStops > MAX_STOPS) goto bad;
    if (fread(stops, sizeof(Stop), nStops, f) != (size_t)nStops) goto bad;
    if (fread(&busesUsed, sizeof busesUsed, 1, f) != 1) goto bad;
    if (fread(&busCapacity, sizeof busCapacity, 1, f) != 1) goto bad;
    fclose(f);
    return;
bad:
    printf("Data file is corrupted, starting fresh.\n");
    nColleges = nStudents = nStops = 0;
    busesUsed = busCapacity = 0;
    fclose(f);
}

College *findCollege(int id) {
    for (int i = 0; i < nColleges; i++)
        if (colleges[i].id == id) return &colleges[i];
    return NULL;
}

Stop *findStop(int id) {
    for (int i = 0; i < nStops; i++)
        if (stops[i].id == id) return &stops[i];
    return NULL;
}

Student *findStudent(int id) {
    for (int i = 0; i < nStudents; i++)
        if (students[i].id == id) return &students[i];
    return NULL;
}

void registerCollege(void) {
    if (nColleges >= MAX_COLLEGES) { printf("College limit reached.\n"); return; }
    College c;
    c.id = nColleges ? colleges[nColleges - 1].id + 1 : 1;
    readLine("College name        : ", c.name, LEN);
    readLine("Coordinator name    : ", c.coordinator, LEN);
    readLine("Coordinator phone   : ", c.phone, sizeof c.phone);
    if (strlen(c.name) == 0) { printf("College name cannot be empty.\n"); return; }
    colleges[nColleges++] = c;
    saveData();
    printf("College registered. College ID = %d\n", c.id);
}

void listColleges(void) {
    printf("\n%-5s %-30s %-20s %-12s\n", "ID", "College", "Coordinator", "Phone");
    for (int i = 0; i < nColleges; i++)
        printf("%-5d %-30s %-20s %-12s\n", colleges[i].id, colleges[i].name,
               colleges[i].coordinator, colleges[i].phone);
    if (!nColleges) printf("(no colleges yet)\n");
}

void addStudent(void) {
    if (nStudents >= MAX_STUDENTS) { printf("Student limit reached.\n"); return; }
    listColleges();
    int cid = readInt("Enter College ID: ");
    if (!findCollege(cid)) { printf("Invalid college ID.\n"); return; }
    Student s;
    memset(&s, 0, sizeof s);
    s.id = nStudents ? students[nStudents - 1].id + 1 : 1;
    s.collegeId = cid;
    readLine("Student name        : ", s.name, LEN);
    readLine("Roll no.            : ", s.roll, sizeof s.roll);
    readLine("Event               : ", s.event, LEN);
    if (strlen(s.name) == 0) { printf("Name cannot be empty.\n"); return; }
    students[nStudents++] = s;
    saveData();
    printf("Student added. Student ID = %d\n", s.id);
}

void listParticipants(void) {
    int cid = readInt("College ID (0 = all colleges): ");
    printf("\n%-5s %-22s %-12s %-18s %-24s\n", "ID", "Name", "Roll", "Event", "College");
    int count = 0;
    for (int i = 0; i < nStudents; i++) {
        if (cid != 0 && students[i].collegeId != cid) continue;
        College *c = findCollege(students[i].collegeId);
        printf("%-5d %-22s %-12s %-18s %-24s\n", students[i].id, students[i].name,
               students[i].roll, students[i].event, c ? c->name : "?");
        count++;
    }
    printf("Total: %d\n", count);
}

void addStop(void) {
    if (nStops >= MAX_STOPS) { printf("Stop limit reached.\n"); return; }
    Stop s;
    s.id = nStops ? stops[nStops - 1].id + 1 : 1;
    readLine("Stop name           : ", s.name, LEN);
    s.order = readInt("Route order (1 = first) : ");
    readLine("Pickup time (HH:MM) : ", s.time, sizeof s.time);
    if (strlen(s.name) == 0) { printf("Stop name cannot be empty.\n"); return; }
    stops[nStops++] = s;
    saveData();
    printf("Stop added. Stop ID = %d\n", s.id);
}

void listStops(void) {
    printf("\n%-5s %-25s %-7s %-8s\n", "ID", "Stop", "Order", "Time");
    for (int i = 0; i < nStops; i++)
        printf("%-5d %-25s %-7d %-8s\n", stops[i].id, stops[i].name,
               stops[i].order, stops[i].time);
    if (!nStops) printf("(no stops yet - add stops first)\n");
}

void transportForm(void) {
    if (!nStops) { printf("Add stops first (menu 4).\n"); return; }
    int sid = readInt("Student ID: ");
    Student *s = findStudent(sid);
    if (!s) { printf("Student not found.\n"); return; }
    printf("Student: %s\n", s->name);
    int need = readInt("Need transport? (1 = Yes, 0 = No): ");
    if (!need) {
        s->needsTransport = 0; s->stopId = 0; s->busNo = 0;
        saveData();
        printf("Saved: no transport required.\n");
        return;
    }
    listStops();
    int stopId = readInt("Enter Stop ID: ");
    if (!findStop(stopId)) { printf("Invalid stop.\n"); return; }
    s->needsTransport = 1;
    s->stopId = stopId;
    s->busNo = 0;
    saveData();
    printf("Transport requirement saved.\n");
}

void sortedStopIndexes(int idx[]) {
    for (int i = 0; i < nStops; i++) idx[i] = i;
    for (int i = 0; i < nStops - 1; i++)
        for (int j = 0; j < nStops - 1 - i; j++)
            if (stops[idx[j]].order > stops[idx[j + 1]].order) {
                int t = idx[j]; idx[j] = idx[j + 1]; idx[j + 1] = t;
            }
}

void allocateBuses(void) {
    if (!nStops) { printf("No stops defined.\n"); return; }
    int cap = readInt("Seats per bus: ");
    if (cap <= 0) { printf("Capacity must be positive.\n"); return; }
    int idx[MAX_STOPS];
    sortedStopIndexes(idx);
    int bus = 1, seatsUsed = 0, total = 0;
    for (int i = 0; i < nStudents; i++) students[i].busNo = 0;
    for (int k = 0; k < nStops; k++) {
        int sid = stops[idx[k]].id;
        for (int i = 0; i < nStudents; i++) {
            if (!students[i].needsTransport || students[i].stopId != sid) continue;
            if (seatsUsed == cap) { bus++; seatsUsed = 0; }
            students[i].busNo = bus;
            seatsUsed++;
            total++;
        }
    }
    busesUsed = total ? bus : 0;
    busCapacity = cap;
    saveData();
    printf("Allocated %d students in %d bus(es).\n", total, busesUsed);
}

void showSummary(void) {
    if (busesUsed == 0) { printf("Nothing allocated yet. Run allocation first (menu 8).\n"); return; }
    int idx[MAX_STOPS];
    sortedStopIndexes(idx);
    printf("\n================ TRANSPORT SUMMARY ================\n");
    for (int b = 1; b <= busesUsed; b++) {
        int count = 0;
        for (int i = 0; i < nStudents; i++) if (students[i].busNo == b) count++;
        printf("\nBUS %d (%d / %d seats)\n", b, count, busCapacity);
        printf("--------------------------------------------------\n");
        for (int k = 0; k < nStops; k++) {
            Stop *st = &stops[idx[k]];
            int c = 0;
            for (int i = 0; i < nStudents; i++)
                if (students[i].busNo == b && students[i].stopId == st->id) c++;
            if (!c) continue;
            printf("Stop: %s | Time: %s | Students: %d\n", st->name, st->time, c);
            for (int i = 0; i < nStudents; i++)
                if (students[i].busNo == b && students[i].stopId == st->id) {
                    College *col = findCollege(students[i].collegeId);
                    printf("  - %-20s (%s)\n", students[i].name, col ? col->name : "?");
                }
        }
    }
    printf("\n===================================================\n");
}

int main(void) {
    loadData();
    while (1) {
        printf("\n===== ATHENA REGISTRATION SYSTEM =====\n");
        printf(" 1. Register college\n");
        printf(" 2. Add student (participation)\n");
        printf(" 3. View colleges\n");
        printf(" 4. Add bus stop\n");
        printf(" 5. Transport requirement form\n");
        printf(" 6. View participation list\n");
        printf(" 7. View stops\n");
        printf(" 8. Allocate buses\n");
        printf(" 9. Show transport summary\n");
        printf(" 0. Exit\n");
        int ch = readInt("Choice: ");
        switch (ch) {
            case 1: registerCollege(); break;
            case 2: addStudent(); break;
            case 3: listColleges(); break;
            case 4: addStop(); break;
            case 5: transportForm(); break;
            case 6: listParticipants(); break;
            case 7: listStops(); break;
            case 8: allocateBuses(); break;
            case 9: showSummary(); break;
            case 0:
                saveData();
                printf("Goodbye\nThanks for visiting\n");
                return 0;
            default: printf("Invalid choice.\n");
        }
    }
}
