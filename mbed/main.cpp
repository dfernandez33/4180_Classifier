#include "mbed.h"
#include "rtos.h"
#include "motordriver.h"
#include "PololuLedStrip.h"
// Connect mbed to Pi USB
Serial  pi(USBTX, USBRX);

Serial blue(p9, p10);

// Set up motors
Motor L(p24, p23, p22, 1); // pwmA, fwd, rev, can brake 
Motor R(p21, p8, p7, 1); // pwmB, fwd, rev, can brake

PololuLedStrip ledStrip(p5);

// for LED strip
Timer timer;

volatile bool disco_mode = false;
volatile bool tornado_mode = false;
volatile bool autonomous_mode = false;

#define LED_COUNT 82
rgb_color colors[LED_COUNT];


// Converts a color from the HSV representation to RGB.
rgb_color hsvToRgb(float h, float s, float v)
{
    int i = floor(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);
    float r = 0, g = 0, b = 0;
    switch(i % 6){
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }
    return (rgb_color){r * 255, g * 255, b * 255};
}

#define TIME_SLICE 5.0f

// LEDs for debugging

// Invalid character was read
DigitalOut led1(LED1);

// Detected tornado
DigitalOut led2(LED2);
DigitalOut led3(LED3);

// Detected disco_ball
DigitalOut led4(LED4);


void forward()
{
    L.speed(0.5);
    R.speed(0.5);
    wait(TIME_SLICE/10);
    L.coast();
    R.coast();
}

void backward()
{
    L.speed(-0.5);
    R.speed(-0.5);
    wait(TIME_SLICE/10);
    L.coast();
    R.coast();
}

void left()
{
    L.speed(-0.5);
    R.speed(0.5);
    wait(TIME_SLICE/10);
    L.coast();
    R.coast();
}

void right()
{
    L.speed(0.5);
    R.speed(-0.5);
    wait(TIME_SLICE/10);
    L.coast();
    R.coast();
}

void stop()
{
    L.stop(0);
    R.stop(0);
}

void led_strip()
{
    timer.start();
    
    while(1)
    {
        if (disco_mode)
        {
            // Update the colors array.
            uint32_t time = timer.read_ms();       
            for(int i = 0; i < LED_COUNT; i++)
            {
                uint8_t phase = (time >> 4) - (i << 2);
                colors[i] = hsvToRgb(phase / 256.0, 1.0, 0.25);
            }
        
            // Send the colors to the LED strip.
            ledStrip.write(colors, LED_COUNT);
        }
        else if (tornado_mode)
        {
            rgb_color temp;
            temp.red = 64;
            temp.green = 64;
            temp.blue = 0;
            for (int i = 0; i < LED_COUNT; i++)
            {
                colors[i] = temp;
            }
            ledStrip.write(colors, LED_COUNT);
        }
        else
        {
            rgb_color temp;
            temp.red = 64;
            temp.green = 0;
            temp.blue = 0;
            for (int i = 0; i < LED_COUNT; i++)
            {
                if (i == LED_COUNT/2)
                {
                    temp.red = 0;
                    temp.green = 64;
                }
                colors[i] = temp;
            }
            ledStrip.write(colors, LED_COUNT);
        }
    }
}

void detected_tornado()
{
    tornado_mode = true;
    for (int i = 0; i < 10; i++)
    {
        left();
        wait(0.5);
    }
    stop();
    tornado_mode = false;
}

void detected_disco_ball()
{
    led4 = 1;
    disco_mode = true;
    left();
    left();
    right();
    right();
    left();
    left();
    right();
    right();
    forward();
    backward();
    disco_mode = false;
    
}

void execute(char c)
{
    switch (c) 
    {
        case 'f':
            forward();
            break;
        case 'b':
            backward();
            break;
        case 'l':
            left();
            break; 
        case 'r':
            right();
            break; 
        case 't':
            detected_tornado();
            break;        
        case 'd':
            detected_disco_ball();
            break;
        case 's':
            stop();
            break; 
        default:
            // invalid character
            led1 = 1;
            break;
    }       
}   

int main()
{
    led1 = 0;
    led2 = 0;
    led3 = 0;
    led4 = 0;
    pi.baud(9600);
    char bnum, bhit;
    
    Thread t1;
    t1.start(led_strip);
    
    while(1) {
        // pi commands are read first
        if(pi.readable()) {
            char c = pi.getc();
            if (c != 'n')
                execute(c);
                switch (c)
                {
                    case 'd':
                    case 't':
                        pi.putc('f');
                        autonomous_mode = false;
                        break;
                    default:
                        pi.putc(c);
                        break;
                }
        }

        // bluetooth commands are read later
        if (!blue.readable()) continue;
        if (blue.getc()=='!') {
            if (blue.getc()=='B') { //button data packet
                bnum = blue.getc(); //button number
                bhit = blue.getc(); //1=hit, 0=release
                if (blue.getc()==char(~('!' + 'B' + bnum + bhit))) { //checksum OK?
                    //myled = bnum - '0'; //current button number will appear on LEDs
                    switch (bnum) {
                        case '1': //number button 1
                            if (bhit=='1') {
                                autonomous_mode = !autonomous_mode;
                                if (!autonomous_mode)
                                {
                                    pi.putc('f');
                                }
                                else
                                {
                                    pi.putc('a');
                                }
                                //add hit code here
                            }
                            break;
                        case '2': //number button 2
                            if (bhit=='1') {
                                // execute('t');
                                // add hit code here
                            } else {
                                //add release code here
                            }
                            break;
                        case '3': //number button 3
                            if (bhit=='1') {
                                //add hit code here
                            } else {
                                //add release code here
                            }
                            break;
                        case '4': //number button 4
                            if (bhit=='1') {
                                //add hit code here
                            } else {
                                //add release code here
                            }
                            break;
                        case '5': //button 5 up arrow
                            if (bhit=='1') {
                                //add hit code here
                                forward();
                            } else {
                                //add release code here
                                stop();
                            }
                            break;
                        case '6': //button 6 down arrow
                            if (bhit=='1') {
                                //add hit code here
                                backward();
                            } else {
                                //add release code here
                                stop();
                            }
                            break;
                        case '7': //button 7 left arrow
                            if (bhit=='1') {
                                //add hit code here
                                left();
                            } else {
                                //add release code here
                                stop();
                            }
                            break;
                        case '8': //button 8 right arrow
                            if (bhit=='1') 
                            {
                                //add hit code here
                                right();
                            } 
                            else 
                            {
                                //add release code here
                                stop();
                            }
                            break;
                        default:
                            // stop();
                            break;
                    }
                }
            }
        }
    }
   
}
