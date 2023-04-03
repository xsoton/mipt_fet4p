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
#define OPT_VG_START 1 // --Vg_start
#define OPT_VG_STOP  2 // --Vg_stop
#define OPT_VG_STEP  3 // --Vg_step
#define OPT_IG_MAX   4 // --Ig_max
#define OPT_VD       5 // --Vd
#define OPT_ID_MAX   6 // --Id_max
#define OPT_T        7 // --time

// The options we understand
static struct argp_option options[] =
{
	{0,0,0,0, "GATE-SOURCE parameters:", 0},
	{"Vg_start", OPT_VG_START, "double", 0, "Start voltage, V   (-5.0  - 5.0, default 0.0 )", 0},
	{"Vg_stop" , OPT_VG_STOP , "double", 0, "Stop voltage, V    (-5.0  - 5.0, default 1.0 )", 0},
	{"Vg_step" , OPT_VG_STEP , "double", 0, "Voltage step, V    (0.001 - 1.0, default 0.1 )", 0},
	{"Ig_max"  , OPT_IG_MAX  , "double", 0, "Maximum current, A (0.001 - 0.1, default 0.01)", 0},
	{0,0,0,0, "DRAIN-SOURCE parameters:", 0},
	{"Vd"      , OPT_VD      , "double", 0, "Voltage, V         (-5.0  - 5.0, default 0.1 )", 0},
	{"Id_max"  , OPT_ID_MAX  , "double", 0, "Maximum current, A (0.001 - 0.1, default  0.01)", 0},
	{0,0,0,0, "Required:", 0},
	{"time"    , OPT_T       , "double", 0, "Scanning delay time, s (0.1 - 10.0)", 0},
	{0,0,0,0, "Common:", 0},
	{0}
};

// parse arguments
struct arguments
{
	int    sample_name_flag;
	char  *sample_name;
	double Vg_start;
	double Vg_stop;
	double Vg_step;
	double Ig_max;
	double Vd;
	double Id_max;
	int    T_flag;
	double T;
};

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	struct arguments *a = state->input;
	double t;

	switch (key)
	{
		case OPT_VG_START:
			t = atof(arg);
			if ((t < -5.0) || (t > 5.0))
			{
				fprintf(stderr, "# E: <Vg_start> is out of range. See \"fet4p --help\"\n");
				return ARGP_ERR_UNKNOWN;
			}
			a->Vg_start = t;
			break;
		case OPT_VG_STOP:
			t = atof(arg);
			if ((t < -5.0) || (t > 5.0))
			{
				fprintf(stderr, "# E: <Vg_stop> is out of range. See \"fet4p --help\"\n");
				return ARGP_ERR_UNKNOWN;
			}
			a->Vg_stop = t;
			break;
		case OPT_VG_STEP:
			t = atof(arg);
			if ((t < 0.001) || (t > 1.0))
			{
				fprintf(stderr, "# E: <Vg_step> is out of range. See \"fet4p --help\"\n");
				return ARGP_ERR_UNKNOWN;
			}
			a->Vg_step = t;
			break;
		case OPT_IG_MAX:
			t = atof(arg);
			if ((t < 0.001) || (t > 0.1))
			{
				fprintf(stderr, "# E: <Ig_max> is out of range. See \"fet4p --help\"\n");
				return ARGP_ERR_UNKNOWN;
			}
			a->Ig_max = t;
			break;
		case OPT_VD:
			t = atof(arg);
			if ((t < -5.0) || (t > 5.0))
			{
				fprintf(stderr, "# E: <Vd> is out of range. See \"fet4p --help\"\n");
				return ARGP_ERR_UNKNOWN;
			}
			a->Vd = t;
			break;
		case OPT_ID_MAX:
			t = atof(arg);
			if ((t < 0.001) || (t > 0.1))
			{
				fprintf(stderr, "# E: <Id_max> is out of range. See \"fet4p --help\"\n");
				return ARGP_ERR_UNKNOWN;
			}
			a->Id_max = t;
			break;
		case OPT_T:
			t = atof(arg);
			if ((t < 0.1) || (t > 10.0))
			{
				fprintf(stderr, "# E: <T> is out of range. See \"fet4p --help\"\n");
				return ARGP_ERR_UNKNOWN;
			}
			a->T = t;
			a->T_flag = 1;
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
#define VG_START 0.0
#define VG_STOP  5.0
#define VG_STEP  0.1
#define IG_MAX   0.01
#define VG       0.5
#define ID_MAX   0.001

// === threads ====
static void *commander(void *);
static void *worker(void *);

// === utils ===
static int get_run();
static void set_run(int run_new);
static double get_time();

// === global variables
static char dir_str[200];
static pthread_rwlock_t run_lock;
static int run;
static char filename_vac[250];
struct arguments arg = {0};

// #define DEBUG

// === program entry point
int main(int argc, char const *argv[])
{
	int ret = 0;
	int status;

	time_t start_time;
	// struct tm start_time_struct;

	pthread_t t_commander;
	pthread_t t_worker;

	// === parse input parameters
	arg.sample_name_flag = 0;
	arg.sample_name = NULL;
	arg.Vg_start = VG_START;
	arg.Vg_stop = VG_STOP;
	arg.Vg_step = VG_STEP;
	arg.Ig_max = IG_MAX;
	arg.Vd = VG;
	arg.Id_max = ID_MAX;
	arg.T_flag = 0;
	arg.T = 0.0;

	status = argp_parse(&argp, argc, argv, 0, 0, &arg);
	if ((status != 0) || (arg.sample_name_flag != 1) || (arg.T_flag != 1))
	{
		fprintf(stderr, "# E: Error while parsing. See \"fet4p --help\"\n");
		ret = -1;
		goto main_exit;
	}

	#ifdef DEBUG
	fprintf(stderr, "sample_name_flag = %d\n" , arg.sample_name_flag);
	fprintf(stderr, "sample_name      = %s\n" , arg.sample_name);
	fprintf(stderr, "Vg_start         = %le\n", arg.Vg_start);
	fprintf(stderr, "Vg_stop          = %le\n", arg.Vg_stop);
	fprintf(stderr, "Vg_step          = %le\n", arg.Vg_step);
	fprintf(stderr, "Ig_max           = %le\n", arg.Ig_max);
	fprintf(stderr, "Vd               = %le\n", arg.Vd);
	fprintf(stderr, "Id_max           = %le\n", arg.Id_max);
	fprintf(stderr, "T_flag           = %d\n" , arg.T_flag);
	fprintf(stderr, "T                = %le\n", arg.T);
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
					"\tq -- exit the program;\n");
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
	double Vg;
	double Ig;
	double Vd;
	double Id;

	double t;

	double voltage;
	int dir;

	FILE  *vac_fp;
	FILE  *gp;
	char   buf[300];
	char  *c;

	dev_fd = fopen(INS_DEV_FILE, "r+");
	if(dev_fd == NULL)
	{
		fprintf(stderr, "# E: Unable to open power supply \"%s\" (%s)\n", INS_DEV_FILE, strerror(ferror(dev_fd)));
		goto worker_dev_open;
	}
	setlinebuf(dev_fd);

	// === init device
	// channel A - Vg
	fprintf(dev_fd, "smua.source.output = smua.OUTPUT_OFF\n");
	fprintf(dev_fd, "smua.source.func = smua.OUTPUT_DCVOLTS\n");
	fprintf(dev_fd, "smua.source.autorangev = smua.AUTORANGE_ON\n");
	fprintf(dev_fd, "smua.source.levelv = 0.0\n");
	fprintf(dev_fd, "smua.source.limiti = %le\n", arg.Ig_max);

	// channel B - Vd
	fprintf(dev_fd, "smub.source.output = smub.OUTPUT_OFF\n");
	fprintf(dev_fd, "smub.source.func = smub.OUTPUT_DCVOLTS\n");
	fprintf(dev_fd, "smub.source.autorangev = smub.AUTORANGE_ON\n");
	fprintf(dev_fd, "smub.source.levelv = %le\n", arg.Vd);
	fprintf(dev_fd, "smub.source.limiti = %le\n", arg.Id_max);

	// === create vac file
	vac_fp = fopen(filename_vac, "w+");
	if(vac_fp == NULL)
	{
		fprintf(stderr, "# E: Unable to open file \"%s\" (%s)\n", filename_vac, strerror(ferror(vac_fp)));
		goto worker_vac_fopen;
	}
	setlinebuf(vac_fp);

	// === write vac header
	r = fprintf(vac_fp,
		"# Measuring of charge carrier mobility of thin films"
			"in field effect transistor structure by the four probe method\n"
		"# Id vs Vg\n"
		"# Date: %04d.%02d.%02d %02d:%02d:%02d\n"
		"# Start parameters:\n"
		"#   sample_name      = %s\n"
		"#   Vg_start         = %le\n"
		"#   Vg_stop          = %le\n"
		"#   Vg_step          = %le\n"
		"#   Ig_max           = %le\n"
		"#   Vd               = %le\n"
		"#   Id_max           = %le\n"
		"#   T                = %le\n"
		"# 1: index\n"
		"# 2: time, s\n"
		"# 3: Vg, V\n"
		"# 4: Ig, A\n"
		"# 5: Vd, V\n"
		"# 6: Id, A\n",
		start_time_struct.tm_year + 1900,
		start_time_struct.tm_mon + 1,
		start_time_struct.tm_mday,
		start_time_struct.tm_hour,
		start_time_struct.tm_min,
		start_time_struct.tm_sec,
		arg.sample_name,
		arg.Vg_start,
		arg.Vg_stop,
		arg.Vg_step,
		arg.Ig_max,
		arg.Vd,
		arg.Id_max,
		arg.T
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
		arg.Vg_start,
		arg.Vg_stop
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
	fprintf(dev_fd, "eventlog.enable = eventlog.ENABLE\n");
	fprintf(dev_fd, "errorqueue.clear()\n");

	dir = ((arg.Vg_stop-arg.Vg_start)/fabs(arg.Vg_stop-arg.Vg_start) > 0.0) ? 1 : -1;

	while(get_run())
	{
		voltage = arg.Vg_start + vac_index * arg.Vg_step * dir;
		if (((dir > 0) && (voltage > arg.Vg_stop)) || ((dir < 0) && (voltage < arg.Vg_stop)))
		{
			set_run(0);
			break;
		}

		fprintf(stderr, "smua.source.levelv = %lf\n", voltage);
		fprintf(dev_fd, "smua.source.levelv = %lf\n", voltage);
		// fprintf(dev_fd, "errorcode, message = errorqueue.next()\n");
		// fprintf(dev_fd, "print(errorcode, message)\n");
		// c = fgets(buf, 300, dev_fd);
		// if (c == NULL)
		// {
		// 	fprintf(stderr, "# E: Unable to read from device (%s)\n", strerror(ferror(dev_fd)));
		// 	set_run(0);
		// 	break;
		// }
		// fprintf(stderr, "#return error: %s\n", buf);

		usleep(arg.T * 1e6);

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
		sscanf(buf, "%lf", &Vg);

		fprintf(dev_fd, "print(smua.measure.i())\n");
		c = fgets(buf, 300, dev_fd);
		if (c == NULL)
		{
			fprintf(stderr, "# E: Unable to read from device (%s)\n", strerror(ferror(dev_fd)));
			set_run(0);
			break;
		}
		sscanf(buf, "%lf", &Ig);

		fprintf(dev_fd, "print(smub.measure.v())\n");
		c = fgets(buf, 300, dev_fd);
		if (c == NULL)
		{
			fprintf(stderr, "# E: Unable to read from device (%s)\n", strerror(ferror(dev_fd)));
			set_run(0);
			break;
		}
		sscanf(buf, "%lf", &Vd);

		fprintf(dev_fd, "print(smub.measure.i())\n");
		c = fgets(buf, 300, dev_fd);
		if (c == NULL)
		{
			fprintf(stderr, "# E: Unable to read from device (%s)\n", strerror(ferror(dev_fd)));
			set_run(0);
			break;
		}
		sscanf(buf, "%lf", &Id);

		r = fprintf(vac_fp, "%d\t%le\t%+le\t%+le\t%+le\t%+le\n",
			vac_index,
			vac_time,
			Vg, Ig, Vd, Id
		);
		if(r < 0)
		{
			fprintf(stderr, "# E: Unable to print to file \"%s\" (%s)\n", filename_vac, strerror(r));
			set_run(0);
			break;
		}

		r = fprintf(gp,
			"set title \"i = %d, t = %.3lf s\"\n"
			"plot \"%s\" u 3:6 w l lw 1 title \"Vg = %.3lf V, Id = %le A\"\n",
			vac_index,
			vac_time,
			filename_vac,
			Vg, Id
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
	fprintf(dev_fd, "smua.source.output = smua.OUTPUT_OFF\n");

	fprintf(dev_fd, "smub.source.levelv = 0.0\n");
	fprintf(dev_fd, "smub.source.output = smua.OUTPUT_OFF\n");

	fprintf(dev_fd, "beeper.beep(1, 440)");

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
