/*
Exists to fetch raw temperature from the physical sensor 
via I2C commands from Linux
*/

#include "sensor.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

float read_hardware_sensor(float offline_fallback){
    //if sensor path is missing, exit
    if (access("/sys/class/i2c-adapter/i2c-1/1-0038/name", F_OK) !=0){
        return offline_fallback;
    }

    //open chip file lane
    int fd=opoen("/dev/i2c-1", O_RDWR);
    if (fd<0){
        return offline_fallback;
    }

    //establish connection using Linux slave macros
    if (ioctl(fd, I2C_SLAVE, 0x38) < 0){
        close(fd);
        return offline_fallback;
    }

    //hardware ingeestion execution
    unsigned char trigger[]={0xAC, 0x33, 0x00};
    write(fd, trigger, 3);
    usleep(80000); //80ms delay

    //unsigned char prevents default C bit bugs
    unsigned char data[7];
    if (read(fd, data, 7) !=7){
        close(fd);
        return offline_fallback;
    }

    //data packing calculation loop
    unsigned int raw=((data[3] & 0x0F) << 16) | (data[4] << 8) | data[5];
    float celsius = ((float)raw / 1048576.0f) * 200.0f - 50.0f;
    
    close(fd);
    return (celsius * 9.0f / 5.0f) + 32.0f;
}

