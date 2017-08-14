#define DegPerRad       57.29577951308232087680
#define RadPerDeg        0.01745329251994329576
#define FALSE 0
#define TRUE  1


typedef struct Vector {
    double x;
    double y;
    double z;
} Vector;


typedef struct Position {
    double x;
    double y;
    double z;
} Position;


typedef struct CTrans {
    double UT;			/* Universal Time (in decimal hours) */
    int    year;		/* 2 digit year */
    int    month;		/* 2 digit month of year */
    int    day;			/* 2 digit day of month */
    int    doy;			/* 3 digit Day Of Year */
    int    dow;			/* 1 digit day of week */
    char   dowstr[80];		/* Day of week String (e.g. "Sun") */
    double gmst; 		/* Greenwich Mean Sidereal Time (in radians) */
    double eccentricity;	/* Eccentricity of Earth-Sun orbit */
    double epsilon;		/* Obliquity of the ecliptic (in radians) */
    double lambda_sun;		/* Ecliptic Long. of Sun (in radians) */
    double earth_sun_dist;	/* Earth-Sun distance (in units of earth radii) */
    double RA_sun;		/* Right Ascention of Sun (in degrees) */
    double DEC_sun;		/* Declination of Sun (in degrees) */
    Vector Sun;			/* direction of Sun in GEI system (unit vector) */
    Vector EcPole;		/* direction of Ecliptic Pole in GEI system (unit vector) */
    double psi;			/* Geodipole tilt angle (in radians) */
    double Dipole_Gcolat;       /* Geographic colat of centered dipole axis (deg.) */
    double Dipole_Glon;         /* Geographic long. of centered dipole axis (deg.) */

    double RA_moon;		/* Right Ascention of Moon (in degrees) */
    double DEC_moon;		/* Declination of Moon (in degrees) */
    double MoonPhase;		/* The Phase of the Moon (in days) */
    double MoonAge;		/* Age of Moon in Days */
    double EarthMoonDistance;	/* Distance between the Earth and Moon (in earth-radii) */
    double Glat;		/* Geographic Latitude of Observer */
    double Glon;		/* Geographic Longitude of Observer */
    double h_moon;		/* Altitude of Moon (in degrees) */
    double A_moon;		/* Azimuth of Moon (in degrees) */
    int	   Visible;		/* Wether or not moon is above horizon */
} CTrans;

void CalcEphem(long int, double, CTrans*);
int  DayofWeek(int, int, int, char*);
int  DayofYear(int, int, int);

