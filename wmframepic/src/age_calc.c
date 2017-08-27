#include "age_calc.h"

static int months_table[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static void calc_day_year_month(time_t today, int day_birth, int month_birth, int year_birth, int *result) {
	int day_curr, month_curr, year_curr, total_days, total_months, total_years;
	struct tm *struct_today;

	struct_today = localtime(&today);

	day_curr = struct_today->tm_mday;
	month_curr = struct_today->tm_mon + 1;
	year_curr = struct_today->tm_year + 1900;

	if((year_curr%4==0 && year_curr%100 !=0) || year_curr%400==0)
		months_table[2] = 29;
	else
		months_table[2] = 28;

	total_years = year_curr - year_birth;
	total_months = month_curr - month_birth;
	total_days = day_curr - day_birth;

	if(total_days < 0) {
		total_days += months_table[month_curr==1 ? 12 : month_curr-1];
		total_months--;
	}

	if(total_months < 0) {
		total_months += 12;
		total_years--;
	}

	result[0] = total_days;
	result[1] = total_months;
	result[2] = total_years;

}

static void construct_phrases(int *result, char **buffer) {
	sprintf(buffer[0], "%02d %s", result[2], result[2] > 1 ? "years" : "year");
	sprintf(buffer[1], "%02d %s", result[1], result[1] > 1 ? "months" : "month");
	sprintf(buffer[2], "%02d %s", result[0], result[0] > 1 ? "days" : "day");

}

static char **alloc_phrases() {
	char **phrases;
	phrases = malloc(NUMBER_OF_ROWS * sizeof(char *));
	int i;

	for(i = 0; i < NUMBER_OF_ROWS; i++)
		phrases[i] = malloc(NUMBER_OF_COLUMNS * sizeof(char));

	return phrases;

}

void clear_phrases(char **phrases) {
	int i;

	for(i = 0; i < NUMBER_OF_ROWS; i++)
		free(phrases[i]);

	free(phrases);

}

char **get_phrases(int day, int month, int year) {
	time_t today;
	time(&today);
	char **phrases = alloc_phrases();
	int result[3];

	calc_day_year_month(today, day, month, year, result);
	construct_phrases(result, phrases);

	return phrases;

}

