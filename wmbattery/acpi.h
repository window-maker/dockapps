/*
 * A not-yet-general-purpose ACPI library, by Joey Hess <joey@kitenet.net>
 */

/* Define ACPI_THERMAL to make the library support finding info about thermal
 * sources. */
/* #define ACPI_THERMAL 1 */

/* Define ACPI_APM to get the acpi_read function, which is like apm_read. */
/* #define ACPI_APM 1 */

/* The lowest version of ACPI proc files supported. */
#define ACPI_VERSION 20011018

/* The number of acpi items of each class supported. */
#define ACPI_MAXITEM 8

int acpi_supported(void);
#ifdef ACPI_APM
int acpi_read(int battery, apm_info *info);
#endif
char *get_acpi_file(const char *file);
int scan_acpi_num(const char *buf, const char *key);
char *scan_acpi_value(const char *buf, const char *key);
char *get_acpi_value(const char *file, const char *key);
int get_acpi_batt_capacity(int battery);

extern int acpi_batt_count;
/* Filenames of the battery info files for each system battery. */
extern char acpi_batt_info[ACPI_MAXITEM][128];
/* Filenames of the battery status files for each system battery. */
extern char acpi_batt_status[ACPI_MAXITEM][128];
/* Stores battery capacity, or 0 if the battery is absent. */
extern int acpi_batt_capacity[ACPI_MAXITEM];

extern int acpi_ac_count;
extern char acpi_ac_adapter_info[ACPI_MAXITEM][128];
extern char acpi_ac_adapter_status[ACPI_MAXITEM][128];

#if ACPI_THERMAL
extern int acpi_thermal_count;
extern char acpi_thermal_info[ACPI_MAXITEM][128];
extern char acpi_thermal_status[ACPI_MAXITEM][128];
#endif

/* This enum is used to index into the acpi_labels */
enum acpi_labels_items {
	label_info,
	label_status,
	label_battery,
	label_ac_adapter,
	label_capacity,
	label_design_capacity,
	label_present,
	label_remaining_capacity,
	label_present_rate,
	label_charging_state,
#if ACPI_THERMAL
	label_thermal,
#endif
	label_ac_state,
	label_last_full_capacity,
};

/* This is set to point to a list of strings used for the given acpi
 *  * version. */
extern char *acpi_labels[];
