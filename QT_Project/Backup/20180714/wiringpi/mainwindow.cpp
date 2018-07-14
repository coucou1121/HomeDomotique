#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //set libraries for GPIO
    //wiringPiSetup();
    //qDebug() << "init Wiringpi SPI" << wiringPiSPISetup(0,500000);
    //qDebug() << "init Wiring I2C" << wiringPiI2CSetup(BME280_I2C_ADRESS);
    //this->readDataSP();
    this->readDataI2C();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readDataSP()
{
    int chan = 0;
    int x = 0;
    double valueVolt = 0;

    unsigned char *data = 0;

    mcp3004Setup (BASE, SPI_CHAN); // 3004 and 3008 are the same 4/8 channels

    while(true)
    {
        for (chan = 0 ; chan < 8 ; ++chan)
        {
            x = analogRead (BASE + chan) ;
            valueVolt = (3.3*x*1000)/1024;
            //  printf("%d\n", x);
            qDebug() << "read values " << chan << ": " << x << "\t volt : " << valueVolt;
        }
        qDebug() << "***************************************" << "\n";
        this->_waitDelay(2);
    }

}

void MainWindow::readDataI2C()
{



    int fd = wiringPiI2CSetup(BME280_ADDRESS);
    if(fd < 0) {
        printf("Device not found");
    }


    BME280cpp *bme280 = new BME280cpp();

    bme280_calib_data cal;
    bme280->readCalibrationData(fd, &cal);

    //    wiringPiI2CWriteReg8(fd, 0xf2, 0x01);   // humidity oversampling x 1
    //    wiringPiI2CWriteReg8(fd, 0xf4, 0x25);   // pressure and temperature oversampling x 1, mode normal

    bme280_raw_data raw;
    bme280->getRawData(fd, &raw);

    int32_t t_fine = bme280->getTemperatureCalibration(&cal, raw.temperature);
    float t = bme280->compensateTemperature(t_fine); // C
    float p = bme280->compensatePressure(raw.pressure, &cal, t_fine) / 100; // hPa
    float h = bme280->compensateHumidity(raw.humidity, &cal, t_fine);       // %
    float a = bme280->getAltitude(p);                         // meters

    for(int i = 0; i<10 ; i++)
    {
        t_fine = bme280->getTemperatureCalibration(&cal, raw.temperature);

        bme280->getRawData(fd, &raw);

        t = bme280->compensateTemperature(t_fine); // C
        p = bme280->compensatePressure(raw.pressure, &cal, t_fine) / 100; // hPa
        h = bme280->compensateHumidity(raw.humidity, &cal, t_fine);       // %
        a = bme280->getAltitude(p);                         // meters

        qDebug() << "***************************";
        qDebug() << "Sample" << i;
        qDebug() << "Temperature" << t << "raw" << raw.temperature;
        qDebug() << "Pressure" << p;
        qDebug() << "Humidity" << h;
        qDebug() << "Altitude" << a;

        _waitDelay(5);
    }
    printf("{\"sensor\":\"bme280\", \"humidity\":%.2f, \"pressure\":%.2f,"
           " \"temperature\":%.2f, \"altitude\":%.2f, \"timestamp\":%d}\n",
           h, p, t, a, (int)time(NULL));

}


void MainWindow::_waitDelay(int delayInSeconde)
{
    QThread::sleep(delayInSeconde);
}
