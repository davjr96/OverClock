#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <malloc.h>
#include <wiringSerial.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <wiringPi.h>
#include <lcd.h>
#include <dirent.h>
#include <fcntl.h>

char* substring(char*, int, int);
double getOutsideTemp();
double getOutsideHumidity();
double getOutsidePressure();
void clearDisplay();
void LEDInit();
void LCDInit();
void setBrightness(int value);
void setDecimals(int decimals);
void ds18b20Init();
double ds18b20Read();

int fd,fd1,fd2;
DIR *dir;
struct dirent *dirent;
char dev[16];      // Dev ID
char devPath[128]; // Path to device
char buf[256];     // Data from device
char tmpData[6];   // Temp C * 1000 reported by device
char path[] = "/sys/bus/w1/devices";
ssize_t numRead;

int main()
{
    struct tm * timeinfo;
    char buffer [80],hour[80], minute[80];
    char LCDBuffer[80];
    time_t rawtime;

    if (wiringPiSetupGpio() == -1)
        return 1;

    LEDInit();
    ds18b20Init();
    LCDInit();

    system("curl -O \"api.openweathermap.org/data/2.5/weather?q=reston&mode=xml\"");
    double outsideTemp = getOutsideTemp();
    double humidity = getOutsideHumidity();
    double pressure = getOutsidePressure();
    system("sudo  rm \"weather?q=reston&mode=xml\"");

    pinMode(24, OUTPUT); //Fan
    digitalWrite(24, LOW);

    for(;;)
    {
        time (&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(buffer,80,"%I%M",timeinfo);

        strftime(hour,80,"%H",timeinfo);
        int h = atoi(hour);

        strftime(minute,80,"%M",timeinfo);
        int m = atoi(minute);

		if ((h >= 21 || h < 8) || (ds18b20Read()>90))
		{
			digitalWrite(24, HIGH);
		}
		else if (ds18b20Read() <80)
		{
			digitalWrite(24, LOW);
		}

		if (h >= 21 || h < 9)
		{
			digitalWrite(25, LOW);
			setBrightness(0);
		}
		else
		{
			digitalWrite(25, HIGH);
			setBrightness(255);
		}

        if (m==0)
        {

			      system("curl -O \"api.openweathermap.org/data/2.5/weather?q=reston&mode=xml\"");
            outsideTemp = getOutsideTemp();
            humidity = getOutsideHumidity();
            pressure = getOutsidePressure();
            system("sudo  rm \"weather?q=reston&mode=xml\"");
        }

       // fflush (stdout) ;
        serialPuts (fd, buffer) ;
        fflush (stdout);

        //lcdClear(fd1);
        lcdHome(fd1);
        sprintf(LCDBuffer, "Outside Temp: %.1lf F",outsideTemp);
        lcdPuts(fd1,LCDBuffer);

        lcdPosition (fd1, 0, 1);
        sprintf(LCDBuffer,"Inside Temp: %.1lf F",ds18b20Read());
        lcdPuts(fd1,LCDBuffer);

        lcdPosition (fd1, 0, 2);
        sprintf(LCDBuffer,"Humidity: %.1lf %%",humidity);
        lcdPuts(fd1,LCDBuffer);

        lcdPosition (fd1, 0, 3);
        sprintf(LCDBuffer,"Pressure: %.1lf hPa",pressure);
        lcdPuts(fd1,LCDBuffer);

        delay(60000);
    }
    return 0;
}
void LCDInit()
{
    fd1 = lcdInit (4, 20, 4, 7,  8, 17,18,27,22,0,0,0,0);
    if (fd1 == -1)
    {
        printf ("lcdInit 1 failed\n");
    }
    pinMode(25, OUTPUT); //Backlight
    digitalWrite(25, HIGH);
    lcdClear(fd1);
    lcdHome(fd1);
}
double ds18b20Read()
{
    float tempC;
    fd2 = open(devPath, O_RDONLY);
    if(fd2 == -1)
    {
        perror ("Couldn't open the w1 device.");
        return 1;
    }
    while((numRead = read(fd2, buf, 256)) > 0)
    {
        strncpy(tmpData, strstr(buf, "t=") + 2, 5);
        tempC = strtof(tmpData, NULL);
    }
    close(fd2);
    return ((tempC / 1000) * 9 / 5 + 32);
}
void ds18b20Init()
{
    system("sudo modprobe w1-gpio");
    system("sudo modprobe w1-therm");

    dir = opendir (path);
    if (dir != NULL)
    {
        while ((dirent = readdir (dir)))

            if (dirent->d_type == DT_LNK && strstr(dirent->d_name, "28-") != NULL)
            {
                strcpy(dev, dirent->d_name);
                printf("\nDevice: %s\n", dev);
            }
        (void) closedir (dir);
    }
    else
    {
        perror ("Couldn't open the w1 devices directory");
    }
    sprintf(devPath, "%s/%s/w1_slave", path, dev);
}
char* substring(char *string, int position, int length)
{
    char *pointer;
    int c;
    pointer = malloc(length + 1);

    if (pointer == NULL)
    {
        printf("Unable to allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    for (c = 0; c < position - 1; c++)
        string++;

   for (c = 0; c < length; c++)
   {
        *(pointer + c) = *string;
        string++;
    }

    *(pointer + c) = '\0';

    return pointer;
}
double getOutsideTemp()
{
    FILE *fp;
    char buff[255];
    char  *pointer;


    fp = fopen("weather?q=reston&mode=xml", "r");


    for (int x =0; x<8; x++)
    {
        fgets(buff, 255, (FILE*)fp);
    }
    pointer = substring(buff, 23, 5);
    double num = atof(pointer);

    num = num - 273;
    num = (num *1.8) + 32;

    free(pointer);
    fclose(fp);
    return num;
}
double getOutsideHumidity()
{
    FILE *fp;
    char buff[255];
    char  *pointer;

    fp = fopen("weather?q=reston&mode=xml", "r");


    for (int x =0; x<9; x++)
    {
        fgets(buff, 255, (FILE*)fp);
    }
    pointer = substring(buff, 20, 3);

    double num = atof(pointer);

    free(pointer);
    fclose(fp);
    return num;

}
double getOutsidePressure()
{
    FILE *fp;
    char buff[255];
    char  *pointer;

    fp = fopen("weather?q=reston&mode=xml", "r");


    for (int x =0; x<10; x++)
    {
        fgets(buff, 255, (FILE*)fp);
    }
    pointer = substring(buff, 19, 5);
	if (pointer[0] = "\"")
	{
	pointer = substring(buff,20,4);
	}

    double num = atof(pointer);

    free(pointer);
    fclose(fp);
    return num;
}
void LEDInit()
{
    if ((fd = serialOpen ("/dev/ttyAMA0", 9600)) < 0)
        fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno));
    clearDisplay();
    setBrightness(255); //full Brightness
    setDecimals(0b00010000); //Only Colon
}

void clearDisplay()
{
    serialPutchar(fd,0x76);
}

void setBrightness(int value)
{
    serialPutchar(fd,0x7A);
    serialPutchar(fd,value);
}

void setDecimals(int decimals)
{
    serialPutchar(fd,0x77);
    serialPutchar(fd,decimals);
}
