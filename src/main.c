#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <argp.h>
#include <error.h>

// === [DATE] ===
struct tm start_time_struct;

// === [ARGUMENTS] ===
const char *argp_program_version = "fet4p 0.1";
const char *argp_program_bug_address = "<killingrain@gmail.com>";
static char doc[] =
	"FET4P -- a program for measuring of charge carrier mobility of "
	"thin films in field effect transistor structure with the use of"
	"four probe method\v"
	"TODO: This part of the documentation comes *after* the options; "
	"note that the text is automatically filled, but it's possible "
	"to force a line-break, e.g.\n<-- here.";
static char args_doc[] = "SAMPLE_NAME";

// Keys for options without short-options
#define OPT_CHAN     1  // --Chan
#define OPT_V1_START 2  // --V1_start
#define OPT_V1_STOP  3  // --V1_stop
#define OPT_V1_STEP  4  // --V1_step
#define OPT_I1_MAX   5  // --I1_max
#define OPT_V2_START 6  // --V2_start
#define OPT_V2_STOP  7  // --V2_stop
#define OPT_V2_STEP  8  // --V2_step
#define OPT_I2_MAX   9  // --I2_max
#define OPT_DELAY    10 // --delay

// The options we understand
static struct argp_option options[] =
{
	{0,0,0,0, "Channels", 0},
	{"Chan"    , OPT_CHAN    , "int"   , 0, "Scanning channel   (1 or 2, default 1 )"       , 0},
	{"V1_start", OPT_V1_START, "double", 0, "Start voltage, V   (-5.0  - 5.0, default 0.0 )", 0},
	{"V1_stop" , OPT_V1_STOP , "double", 0, "Stop voltage, V    (-5.0  - 5.0, default 1.0 )", 0},
	{"V1_step" , OPT_V1_STEP , "double", 0, "Voltage step, V    (0.001 - 1.0, default 0.1 )", 0},
	{"I1_max"  , OPT_I1_MAX  , "double", 0, "Maximum current, A (0.001 - 0.1, default 0.01)", 0},
	{"V2_start", OPT_V2_START, "double", 0, "Start voltage, V   (-5.0  - 5.0, default 0.0 )", 0},
	{"V2_stop" , OPT_V2_STOP , "double", 0, "Stop voltage, V    (-5.0  - 5.0, default 1.0 )", 0},
	{"V2_step" , OPT_V2_STEP , "double", 0, "Voltage step, V    (0.001 - 1.0, default 0.1 )", 0},
	{"I2_max"  , OPT_I2_MAX  , "double", 0, "Maximum current, A (0.001 - 0.1, default 0.01)", 0},
	{0,0,0,0, "Required:", 0},
	{"delay"    , OPT_DELAY  , "double", 0, "Scanning delay time, s (0.1 - 10.0)"           , 0},
	{0,0,0,0, "Common:", 0},
	{0}
};

// parse arguments
struct arguments
{
	int    sample_name_flag;
	char  *sample_name;
	int    Chan;
	double V1_start;
	double V1_stop;
	double V1_step;
	double I1_max;
	double V2_start;
	double V2_stop;
	double V2_step;
	double I2_max;
	int    Delay_flag;
	double Delay;
};

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	struct arguments *a = state->input;
	double t;
	int i;

	switch (key)
	{
	case OPT_CHAN:
			i = atoi(arg);
			if ((i != 1) && (i != 2))
			{
				fprintf(stderr, "# E: <Chan> is out of range. See \"fet4p --help\"\n");
				return ARGP_ERR_UNKNOWN;
			}
			a->Chan = i;
			break;
		case OPT_V1_START:
			t = atof(arg);
			if ((t < -5.0) || (t > 5.0))
			{
				fprintf(stderr, "# E: <V1_start> is out of range. See \"fet4p --help\"\n");
				return ARGP_ERR_UNKNOWN;
			}
			a->V1_start = t;
			break;
		case OPT_V1_STOP:
			t = atof(arg);
			if ((t < -5.0) || (t > 5.0))
			{
				fprintf(stderr, "# E: <V1_stop> is out of range. See \"fet4p --help\"\n");
				return ARGP_ERR_UNKNOWN;
			}
			a->V1_stop = t;
			break;
		case OPT_V1_STEP:
			t = atof(arg);
			if ((t < 0.001) || (t > 1.0))
			{
				fprintf(stderr, "# E: <V1_step> is out of range. See \"fet4p --help\"\n");
				return ARGP_ERR_UNKNOWN;
			}
			a->V1_step = t;
			break;
		case OPT_I1_MAX:
			t = atof(arg);
			if ((t < 0.001) || (t > 0.1))
			{
				fprintf(stderr, "# E: <I1_max> is out of range. See \"fet4p --help\"\n");
				return ARGP_ERR_UNKNOWN;
			}
			a->I1_max = t;
			break;
		case OPT_V2_START:
			t = atof(arg);
			if ((t < -5.0) || (t > 5.0))
			{
				fprintf(stderr, "# E: <V2_start> is out of range. See \"fet4p --help\"\n");
				return ARGP_ERR_UNKNOWN;
			}
			a->V2_start = t;
			break;
		case OPT_V2_STOP:
			t = atof(arg);
			if ((t < -5.0) || (t > 5.0))
			{
				fprintf(stderr, "# E: <V2_stop> is out of range. See \"fet4p --help\"\n");
				return ARGP_ERR_UNKNOWN;
			}
			a->V2_stop = t;
			break;
		case OPT_V2_STEP:
			t = atof(arg);
			if ((t < 0.001) || (t > 1.0))
			{
				fprintf(stderr, "# E: <V2_step> is out of range. See \"fet4p --help\"\n");
				return ARGP_ERR_UNKNOWN;
			}
			a->V2_step = t;
			break;
		case OPT_I2_MAX:
			t = atof(arg);
			if ((t < 0.001) || (t > 0.1))
			{
				fprintf(stderr, "# E: <I2_max> is out of range. See \"fet4p --help\"\n");
				return ARGP_ERR_UNKNOWN;
			}
			a->I2_max = t;
			break;
		case OPT_DELAY:
			t = atof(arg);
			if ((t < 0.1) || (t > 10.0))
			{
				fprintf(stderr, "# E: <Delay> is out of range. See \"fet4p --help\"\n");
				return ARGP_ERR_UNKNOWN;
			}
			a->Delay = t;
			a->Delay_flag = 1;
			break;
		case ARGP_KEY_ARG:
			a->sample_name = arg;
			a->sample_name_flag = 1;
			break;
		case ARGP_KEY_NO_ARGS:
			fprintf(stderr, "# E: <sample_name> has not specified. See \"fet4p --help\"\n");
			a->sample_name_flag = 0;
			//argp_usage (state);
			return ARGP_ERR_UNKNOWN;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc, NULL, NULL, NULL};

// === [INSTRUMENT] ===
#define INS_DEV_FILE "/dev/usbtmc0"

// === [SOURCE] ===
#define CHAN     1
#define V1_START 0.0
#define V1_STOP  1.0
#define V1_STEP  0.1
#define I1_MAX   0.01
#define V2_START 0.0
#define V2_STOP  1.0
#define V2_STEP  0.1
#define I2_MAX   0.01

// === threads ====
static void *commander(void *);
static void *worker(void *);

// === utils ===
static int get_run();
static void set_run(int run_new);
static int get_next();
static void set_next(int next_new);
static double get_time();

static int direction(double start, double stop);

// === global variables
static char dir_str[200];
static pthread_rwlock_t run_lock;
static int run;
static pthread_rwlock_t next_lock;
static int next;
static char filename_vac[250];
struct arguments arg = {0};

// === measurements ===
enum meas_state
{
	M_BEFORE = 0,
	M_STAGE1,
	M_STAGE2,
	M_STAGE3,
	M_AFTER,
	M_STOP
};

// #define DEBUG

// === program entry point
int main(int argc, char const *argv[])
{
	int ret = 0;
	int status;

	time_t start_time;
	struct tm start_time_struct;

	pthread_t t_commander;
	pthread_t t_worker;

	// === parse input parameters
	arg.sample_name_flag = 0;
	arg.sample_name      = NULL;
	arg.Chan             = CHAN;
	arg.V1_start         = V1_START;
	arg.V1_stop          = V1_STOP;
	arg.V1_step          = V1_STEP;
	arg.I1_max           = I1_MAX;
	arg.V2_start         = V2_START;
	arg.V2_stop          = V2_STOP;
	arg.V2_step          = V2_STEP;
	arg.I2_max           = I2_MAX;
	arg.Delay_flag       = 0;
	arg.Delay            = 0.0;

	status = argp_parse(&argp, argc, argv, 0, 0, &arg);
	if ((status != 0) || (arg.sample_name_flag != 1) || (arg.Delay_flag != 1))
	{
		fprintf(stderr, "# E: Error while parsing. See \"fet4p --help\"\n");
		ret = -1;
		goto main_exit;
	}

	#ifdef DEBUG
	fprintf(stderr, "sample_name_flag = %d\n" , arg.sample_name_flag);
	fprintf(stderr, "sample_name      = %s\n" , arg.sample_name);
	fprintf(stderr, "Chan             = %d\n" , arg.Chan);
	fprintf(stderr, "V1_start         = %le\n", arg.V1_start);
	fprintf(stderr, "V1_stop          = %le\n", arg.V1_stop);
	fprintf(stderr, "V1_step          = %le\n", arg.V1_step);
	fprintf(stderr, "I1_max           = %le\n", arg.I1_max);
	fprintf(stderr, "V1_start         = %le\n", arg.V2_start);
	fprintf(stderr, "V1_stop          = %le\n", arg.V2_stop);
	fprintf(stderr, "V1_step          = %le\n", arg.V2_step);
	fprintf(stderr, "I1_max           = %le\n", arg.I2_max);
	fprintf(stderr, "Delay_flag       = %d\n" , arg.Delay_flag);
	fprintf(stderr, "Delay            = %le\n", arg.Delay);
	#endif

	// === get start time of experiment ===
	start_time = time(NULL);
	localtime_r(&start_time, &start_time_struct);

	// === we need actual information w/o buffering
	setlinebuf(stdout);
	setlinebuf(stderr);

	// === initialize run state variable
	pthread_rwlock_init(&run_lock, NULL);
	run = 1;

	// === initialize next state variable
	pthread_rwlock_init(&next_lock, NULL);
	next = 0;

	// === create dirictory in "20191012_153504_<experiment_name>" format
	snprintf(dir_str, 200, "%04d-%02d-%02d_%02d-%02d-%02d_%s",
		start_time_struct.tm_year + 1900,
		start_time_struct.tm_mon + 1,
		start_time_struct.tm_mday,
		start_time_struct.tm_hour,
		start_time_struct.tm_min,
		start_time_struct.tm_sec,
		arg.sample_name
	);
	status = mkdir(dir_str, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if (status == -1)
	{
		fprintf(stderr, "# E: unable to create experiment directory (%s)\n", strerror(errno));
		ret = -2;
		goto main_exit;
	}

	// === create file names
	snprintf(filename_vac, 250, "%s/vac.dat", dir_str);
	// printf("filename_vac \"%s\"\n", filename_vac);

	// === now start threads
	pthread_create(&t_commander, NULL, commander, NULL);
	pthread_create(&t_worker, NULL, worker, NULL);

	// === and wait ...
	pthread_join(t_worker, NULL);

	// === cancel commander thread becouse we don't need it anymore
	// === and wait for cancelation finish
	pthread_cancel(t_commander);
	pthread_join(t_commander, NULL);

	fprintf(stdout, "\r\n");

	main_exit:
	return ret;
}

// === commander function
static void *commander(void *a)
{
	(void) a;

	char str[100];
	char *s;
	int ccount;

	while(get_run())
	{
		fprintf(stdout, "> ");

		s = fgets(str, 100, stdin);
		if (s == NULL)
		{
			fprintf(stderr, "# E: Exit\n");
			set_run(0);
			break;
		}

		switch(str[0])
		{
			case 'h':
				printf(
					"Help:\n"
					"\th -- this help;\n"
					"\tn -- next stage;\n"
					"\tq -- exit the program;\n");
				break;
			case 'n':
				set_next(1);
				break;
			case 'q':
				set_run(0);
				break;
			default:
				ccount = strlen(str)-1;
				fprintf(stderr, "# E: Unknown command (%.*s)\n", ccount, str);
				break;
		}
	}

	return NULL;
}

// === worker function
static void *worker(void *a)
{
	(void) a;

	int r;

	FILE *dev_fd;

	int    vac_index;
	double vac_time;
	double V1, I1, V2, I2;

	double t;

	double voltage;
	int dir;

	FILE  *vac_fp;
	FILE  *gp;
	char   buf[300];
	char  *c;

	enum meas_state state = M_BEFORE;
	int i1, i2, i3;

	double V_start, V_stop, V_step;

	V_start = (arg.Chan == 1) ? arg.V1_start : arg.V2_start;
	V_stop  = (arg.Chan == 1) ? arg.V1_stop  : arg.V2_stop;
	V_step  = (arg.Chan == 1) ? arg.V1_step  : arg.V2_step;

	dev_fd = fopen(INS_DEV_FILE, "r+");
	if(dev_fd == NULL)
	{
		fprintf(stderr, "# E: Unable to open power supply \"%s\" (%s)\n", INS_DEV_FILE, strerror(ferror(dev_fd)));
		goto worker_dev_open;
	}
	setlinebuf(dev_fd);

	// === init device
	// channel A - V1
	fprintf(dev_fd, "smua.source.output = smua.OUTPUT_OFF\n");
	fprintf(dev_fd, "smua.source.func = smua.OUTPUT_DCVOLTS\n");
	fprintf(dev_fd, "smua.source.autorangev = smua.AUTORANGE_ON\n");
	fprintf(dev_fd, "smua.source.levelv = 0.0\n");
	fprintf(dev_fd, "smua.source.limiti = %le\n", arg.I1_max);

	// channel B - V2
	fprintf(dev_fd, "smub.source.output = smub.OUTPUT_OFF\n");
	fprintf(dev_fd, "smub.source.func = smub.OUTPUT_DCVOLTS\n");
	fprintf(dev_fd, "smub.source.autorangev = smub.AUTORANGE_ON\n");
	fprintf(dev_fd, "smub.source.levelv = 0.0\n");
	fprintf(dev_fd, "smub.source.limiti = %le\n", arg.I2_max);

	// === create vac file
	vac_fp = fopen(filename_vac, "w+");
	if(vac_fp == NULL)
	{
		fprintf(stderr, "# E: Unable to open file \"%s\" (%s)\n", filename_vac, strerror(ferror(vac_fp)));
		goto worker_vac_fopen;
	}
	setlinebuf(vac_fp);

	fprintf(stderr, "1\n");

	// === write vac header
	r = fprintf(vac_fp,
		"# Measuring of charge carrier mobility of thin films"
			"in field effect transistor structure by the four probe method\n"
		"# Id vs Vg\n"
		"# Date: %04d.%02d.%02d %02d:%02d:%02d\n"
		"# Start parameters:\n"
		"#   sample_name      = %s\n"
		"#   Chan             = %d\n"
		"#   V1_start         = %le\n"
		"#   V1_stop          = %le\n"
		"#   V1_step          = %le\n"
		"#   I1_max           = %le\n"
		"#   V2_start         = %le\n"
		"#   V2_stop          = %le\n"
		"#   V2_step          = %le\n"
		"#   I2_max           = %le\n"
		"#   Delay            = %le\n"
		"# 1: index\n"
		"# 2: time, s\n"
		"# 3: V1, V\n"
		"# 4: I1, A\n"
		"# 5: V2, V\n"
		"# 6: I2, A\n",
		start_time_struct.tm_year + 1900,
		start_time_struct.tm_mon + 1,
		start_time_struct.tm_mday,
		start_time_struct.tm_hour,
		start_time_struct.tm_min,
		start_time_struct.tm_sec,
		arg.sample_name,
		arg.Chan,
		arg.V1_start,
		arg.V1_stop,
		arg.V1_step,
		arg.I1_max,
		arg.V2_start,
		arg.V2_stop,
		arg.V2_step,
		arg.I2_max,
		arg.Delay
	);
	if(r < 0)
	{
		fprintf(stderr, "# E: Unable to print to file \"%s\" (%s)\n", filename_vac, strerror(r));
		goto worker_vac_header;
	}

	// === open gnuplot
	snprintf(buf, 300, "gnuplot > %s/gnuplot.log 2>&1", dir_str);
	gp = popen(buf, "w");
	if (gp == NULL)
	{
		fprintf(stderr, "# E: unable to open gnuplot pipe (%s)\n", strerror(errno));
		goto worker_gp_popen;
	}
	setlinebuf(gp);

	// === prepare gnuplot
	r = fprintf(gp,
		"set term qt noraise\n"
		"set xzeroaxis lt -1\n"
		"set yzeroaxis lt -1\n"
		"set grid\n"
		"set key right bottom\n"
		"set xrange [%le:%le]\n"
		"set xlabel \"Vg, V\"\n"
		"set ylabel \"Id, A\"\n"
		"set format y \"%%.3s%%c\"\n",
		V_start,
		V_stop
	);
	if(r < 0)
	{
		fprintf(stderr, "# E: Unable to print to gp (%s)\n", strerror(r));
		goto worker_gp_settings;
	}

	// === let the action begins!
	vac_index = 0;

	fprintf(dev_fd, "smua.source.output = smua.OUTPUT_ON\n");
	fprintf(dev_fd, "smub.source.output = smub.OUTPUT_ON\n");

	if (arg.Chan == 1)
		fprintf(dev_fd, "smub.source.levelv = %lf\n", arg.V2_start);
	else
		fprintf(dev_fd, "smua.source.levelv = %lf\n", arg.V1_start);

	if (fabs(V_start) < V_step)
		state = M_STAGE2;
	else
		state = M_STAGE1;

	i1 = i2 = i3 = 0;


// set_run(0);
	while(get_run())
	{
		switch(state)
		{
			case M_STAGE1:
				dir = direction(0.0, V_start);
				if (get_next())
				{
					V_start = voltage + V_step * dir;
					state = M_STAGE2;
					set_next(0);
				}
				else
				{
					voltage = 0.0 + i1 * V_step * dir;
					if (((dir > 0) && (voltage >= V_start)) || ((dir < 0) && (voltage <= V_start)))
						state = M_STAGE2;
					else
					{
						i1++;
						vac_index++;
						break;
					}
				}
			case M_STAGE2:
				dir = direction(V_start, V_stop);
				if (get_next())
				{
					V_stop = voltage + V_step * dir;
					state = M_STAGE3;
					set_next(0);
				}
				else
				{
					voltage = V_start + i2 * V_step * dir;
					if (((dir > 0) && (voltage >= V_stop)) || ((dir < 0) && (voltage <= V_stop)))
						state = M_STAGE3;
					else
					{
						i2++;
						vac_index++;
						break;
					}
				}
			case M_STAGE3:
				if (get_next())
				{
					state = M_AFTER;
					set_next(0);
					break;
				}
				else
				{
					dir = direction(V_stop, 0.0);
					voltage = V_stop + i3 * V_step * dir;
					if (((dir > 0) && (voltage >= 0.0)) || ((dir < 0) && (voltage <= 0.0)))
						state = M_AFTER;
					else
					{
						i3++;
						vac_index++;
						break;
					}
				}
			default:
				state = M_STOP;
		}

		if (state > M_STAGE3)
		{
			set_run(0);
			break;
		}

		fprintf(stderr, "voltage = %lf\n", voltage);

		if (arg.Chan == 1)
			fprintf(dev_fd, "smua.source.levelv = %lf\n", voltage);
		else
			fprintf(dev_fd, "smub.source.levelv = %lf\n", voltage);

		usleep(arg.Delay * 1e6);

		vac_time = get_time();
		if (vac_time < 0)
		{
			fprintf(stderr, "# E: Unable to get time\n");
			set_run(0);
			break;
		}

		fprintf(dev_fd, "print(smua.measure.v())\n");
		c = fgets(buf, 300, dev_fd);
		if (c == NULL)
		{
			fprintf(stderr, "# E: Unable to read from device (%s)\n", strerror(ferror(dev_fd)));
			set_run(0);
			break;
		}
		sscanf(buf, "%lf", &V1);

		fprintf(dev_fd, "print(smua.measure.i())\n");
		c = fgets(buf, 300, dev_fd);
		if (c == NULL)
		{
			fprintf(stderr, "# E: Unable to read from device (%s)\n", strerror(ferror(dev_fd)));
			set_run(0);
			break;
		}
		sscanf(buf, "%lf", &I1);

		fprintf(dev_fd, "print(smub.measure.v())\n");
		c = fgets(buf, 300, dev_fd);
		if (c == NULL)
		{
			fprintf(stderr, "# E: Unable to read from device (%s)\n", strerror(ferror(dev_fd)));
			set_run(0);
			break;
		}
		sscanf(buf, "%lf", &V2);

		fprintf(dev_fd, "print(smub.measure.i())\n");
		c = fgets(buf, 300, dev_fd);
		if (c == NULL)
		{
			fprintf(stderr, "# E: Unable to read from device (%s)\n", strerror(ferror(dev_fd)));
			set_run(0);
			break;
		}
		sscanf(buf, "%lf", &I2);

		r = fprintf(vac_fp, "%d\t%le\t%+le\t%+le\t%+le\t%+le\n",
			vac_index,
			vac_time,
			V1, I1, V2, I2
		);
		if(r < 0)
		{
			fprintf(stderr, "# E: Unable to print to file \"%s\" (%s)\n", filename_vac, strerror(r));
			set_run(0);
			break;
		}

		r = fprintf(gp, "set title \"i = %d, t = %.3lf s\"\n", vac_index, vac_time);
		r = fprintf(gp,
			"plot \"%s\" u %d:4 w l lw 1 title \"V1 = %.3lf V, I1 = %le A\", "
			       "\"\" u %d:6 w l lw 1 title \"V2 = %.3lf V, I2 = %le A\"\n",
			filename_vac,
			(arg.Chan == 1) ? 3 : 5, V1, I1,
			(arg.Chan == 1) ? 3 : 5, V2, I2
		);
		if(r < 0)
		{
			fprintf(stderr, "# E: Unable to print to gp (%s)\n", strerror(r));
			set_run(0);
			break;
		}

		vac_index++;
	}

	fprintf(dev_fd, "smua.source.levelv = 0.0\n");
	fprintf(dev_fd, "smub.source.levelv = 0.0\n");
	usleep(1e6);
	fprintf(dev_fd, "smua.source.output = smua.OUTPUT_OFF\n");
	fprintf(dev_fd, "smub.source.output = smua.OUTPUT_OFF\n");

	fprintf(dev_fd, "beeper.beep(0.15, 220.0)");
	fprintf(dev_fd, "beeper.beep(0.15, 130.8)");
	fprintf(dev_fd, "beeper.beep(0.30, 146.8)");
	fprintf(dev_fd, "beeper.beep(0.30, 146.8)");
	fprintf(dev_fd, "beeper.beep(0.15, 146.8)");
	fprintf(dev_fd, "beeper.beep(0.15, 164.8)");
	fprintf(dev_fd, "beeper.beep(0.30, 174.6)");
	fprintf(dev_fd, "beeper.beep(0.30, 174.6)");
	fprintf(dev_fd, "beeper.beep(0.15, 174.6)");
	fprintf(dev_fd, "beeper.beep(0.15, 196.0)");
	fprintf(dev_fd, "beeper.beep(0.30, 164.8)");
	fprintf(dev_fd, "beeper.beep(0.30, 164.8)");
	fprintf(dev_fd, "beeper.beep(0.15, 146.8)");
	fprintf(dev_fd, "beeper.beep(0.15, 130.8)");
	fprintf(dev_fd, "beeper.beep(0.15, 130.8)");
	fprintf(dev_fd, "beeper.beep(0.30, 146.8)");

	r = fprintf(gp, "exit;\n");
	if(r < 0)
	{
		fprintf(stderr, "# E: Unable to print to gp (%s)\n", strerror(r));
	}

	worker_gp_settings:

	r = pclose(gp);
	if (r == -1)
	{
		fprintf(stderr, "# E: Unable to close gnuplot pipe (%s)\n", strerror(errno));
	}
	worker_gp_popen:


	worker_vac_header:

	r = fclose(vac_fp);
	if (r == EOF)
	{
		fprintf(stderr, "# E: Unable to close file \"%s\" (%s)\n", filename_vac, strerror(errno));
	}
	worker_vac_fopen:

	fclose(dev_fd);
	worker_dev_open:

	return NULL;
}

// === utils
static int get_run()
{
	int run_local;
	pthread_rwlock_rdlock(&run_lock);
		run_local = run;
	pthread_rwlock_unlock(&run_lock);
	return run_local;
}

static void set_run(int run_new)
{
	pthread_rwlock_wrlock(&run_lock);
		run = run_new;
	pthread_rwlock_unlock(&run_lock);
}

static int get_next()
{
	int next_local;
	pthread_rwlock_rdlock(&next_lock);
		next_local = next;
	pthread_rwlock_unlock(&next_lock);
	return next_local;
}

static void set_next(int next_new)
{
	pthread_rwlock_wrlock(&next_lock);
		next = next_new;
	pthread_rwlock_unlock(&next_lock);
}

static double get_time()
{
	static int first = 1;
	static struct timeval t_first = {0};
	struct timeval t = {0};
	double ret;
	int r;

	if (first == 1)
	{
		r = gettimeofday(&t_first, NULL);
		if (r == -1)
		{
			fprintf(stderr, "# E: unable to get time (%s)\n", strerror(errno));
			ret = -1;
		}
		else
		{
			ret = 0.0;
			first = 0;
		}
	}
	else
	{
		r = gettimeofday(&t, NULL);
		if (r == -1)
		{
			fprintf(stderr, "# E: unable to get time (%s)\n", strerror(errno));
			ret = -2;
		}
		else
		{
			ret = (t.tv_sec - t_first.tv_sec) * 1e6 + (t.tv_usec - t_first.tv_usec);
			ret /= 1e6;
		}
	}

	return ret;
}

static int direction(double start, double stop)
{
	return (stop >= start) ? 1 : -1;
}
