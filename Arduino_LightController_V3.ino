/*
 Name:		Arduino_LightController3.ino
 Created:	12/22/2019 5:12:05 PM
 Author:	HAL 9000
*/

#include <ClickButton.h>

#pragma region DefineIO

//define states of the program
enum MainStates {White, Red, Green, Blue, Fading, Off};
MainStates State;

//define output pins of arduino
const int RED = 9;
const int GREEN = 10;
const int BLUE = 11;

//define input pins of arduino
//poti from 0 - 1023
const int POTIpin = A0;
//convert POTI-value into 0 - 255
long potivalue;


//buttons 
//on off button
const int onoffpin = 2;
ClickButton ONOFF(onoffpin, false, CLICKBTN_PULLUP);
bool onflag = false;

//nightmode button
const int nightpin = 3;
const int nightvalue = 7;
ClickButton NIGHT(nightpin, false, CLICKBTN_PULLUP);
bool nightflag = false;

//change mode button
const int changepin = 4;
ClickButton Change(changepin, false, CLICKBTN_PULLUP);

//fading button
const int fadingpin = 5;
ClickButton FADING(fadingpin, false, CLICKBTN_PULLUP);
bool fadingflag = false;


//stuff needed for fading
int fadingspeed = 15;
const int LedOut[3] = { RED, GREEN, BLUE };	//Array fuer gemeinsame manipulation

//flag for entering a state first time to fade into it 
bool firsttimeentering = false;

#pragma endregion

//-------------------------------------------------------------------------------------------------------

//define states
#pragma region States

static void State_Off()
{
    //set everything to zero
    digitalWrite(RED, LOW);
    digitalWrite(GREEN, LOW);
    digitalWrite(BLUE, LOW);

    State = White;
}

static void State_White(int value)
{
    if (firsttimeentering)
    {
        for (int i = 0; i <= value; i++)
        {
            analogWrite(RED, i);
            analogWrite(GREEN, i);
            analogWrite(BLUE, value);;
            delay(fadingspeed);
        }
    }
    firsttimeentering = false;

    analogWrite(RED, value);
    analogWrite(GREEN, value);
    analogWrite(BLUE, value);
}

static void State_Red(int value)
{   
    if (firsttimeentering)
    {
        for (int i = value; i >= 0; i--)
        {
            analogWrite(RED, value);
            analogWrite(GREEN, i);
            analogWrite(BLUE, i);
            delay(fadingspeed);
        }
    }
    firsttimeentering = false;

    analogWrite(RED, value);
    digitalWrite(GREEN, LOW);
    digitalWrite(BLUE, LOW);
}

static void State_Green(int value)
{
    if (firsttimeentering)
    {
        int j = 0;
        for (int i = value; i >= 0; i--)
        {
            analogWrite(RED, i);
            analogWrite(GREEN, j);
            j++;
            delay(fadingspeed);
        }
    }
    firsttimeentering = false;

    digitalWrite(RED, LOW);
    analogWrite(GREEN, value);
    digitalWrite(BLUE, LOW);
}

static void State_Blue(int value)
{
    if (firsttimeentering)
    {
        int j = 0;
        for (int i = value; i >= 0; i--)
        {
            analogWrite(GREEN, i);
            analogWrite(BLUE, j);
            j++;
            delay(fadingspeed);
        }
    }
    firsttimeentering = false;

    digitalWrite(RED, LOW);
    digitalWrite(GREEN, LOW);
    analogWrite(BLUE, value);
}

static void State_Fading()
{

    bool firstround = true;

    while(fadingflag)
    {
        CheckFading();

        //cycle through each colour starting with red with 
        int j = potivalue;
        for (int i = 0; i <= potivalue; i++)
        {   
            CheckFading();
            if (fadingflag)
            {
                if (!firstround)
                    analogWrite(BLUE, j);
                analogWrite(RED, i);
                j--;
                delay(fadingspeed);
            }else
                break;
        }
        if (!fadingflag)
            break;

        j = potivalue;
        for (int i = 0; i <= potivalue; i++)
        {
            CheckFading();
            if (fadingflag)
            {
                analogWrite(RED, j);
                analogWrite(GREEN, i);
                j--;
                delay(fadingspeed);
            }
            else
                break;
        }
        if (!fadingflag)
            break;

        j = potivalue;
        for (int i = 0; i <= potivalue; i++)
        {
            CheckFading();
            if (fadingflag)
            {
                analogWrite(GREEN, j);
                analogWrite(BLUE, i);
                j--;
                delay(fadingspeed);
            }
            else
                break;
        }
        if (!fadingflag)
            break;

        firstround = false;
    }

    State = White;
}

static void CheckFading()
{
    FADING.Update();
    if (FADING.changed)
        fadingflag = false;

    potivalue = map(analogRead(POTIpin), 0, 1023, 0, 255);
}

#pragma endregion


//-------------------------------------------------------------------------------------------------------

// the setup function runs once when you press reset or power the board
void setup() {

    Serial.begin(1000000);

    //set output pins of arduino board
    pinMode(RED, OUTPUT);
    pinMode(GREEN, OUTPUT);
    pinMode(BLUE, OUTPUT);
    
    //initialize them with state off
    State = Off;

    //set input pins of arduino board
    //pinMode(onoffpin, INPUT_PULLUP);
    //pinMode(nightpin, INPUT_PULLUP);
    //pinMode(changepin, INPUT_PULLUP);
    //pinMode(unused, INPUT_PULLUP);

}//end of setup

//-------------------------------------------------------------------------------------------------------

// the loop function runs over and over again until power down or reset
void loop() {

    //update all buttons
    ONOFF.Update();
    NIGHT.Update();
    Change.Update();
    FADING.Update();


    //convert current poti value for the brightness
    potivalue = map(analogRead(POTIpin), 0, 1023, 0, 255);
    
    if (ONOFF.changed)
        onflag = !onflag;


    if (onflag)
    {
        //check for nightmode
        if (NIGHT.changed)
            nightflag = !nightflag;

        //check for fading
        if (FADING.changed && !fadingflag)
            State = Fading;


        if (!nightflag)
        {
            switch (State)
            {
            case White:
                State_White(potivalue);
                fadingflag = false;
                if (Change.changed)
                {
                    State = Red;
                    firsttimeentering = true;
                }
                break;
            case Red:
                State_Red(potivalue);
                fadingflag = false;
                if (Change.changed)
                {
                    State = Green;
                    firsttimeentering = true;
                }
                break;
            case Green:
                State_Green(potivalue);
                fadingflag = false;
                if (Change.changed)
                {
                    State = Blue;
                    firsttimeentering = true;
                }
                break;
            case Blue:
                State_Blue(potivalue);
                fadingflag = false;
                if (Change.changed)
                {
                    State = White;
                    firsttimeentering = true;
                }
                break;
            case Fading:
                fadingflag = true;
                State_Fading();
                break;

            case Off:
                State_Off();
                break;
            default:
                break;
            }
        }
        else //nightmode
        {
            switch (State)
            {
            case White:
                State_White(nightvalue);
                if (Change.changed)
                {
                    State = Red;
                    firsttimeentering = true;
                }
                    break;
            case Red:
                State_Red(nightvalue);
                if (Change.changed)
                {
                    State = Green;
                    firsttimeentering = true;
                }
                break;
            case Green:
                State_Green(nightvalue);
                if (Change.changed)
                {
                    State = Blue;
                    firsttimeentering = true;
                }
                break;
            case Blue:
                State_Blue(nightvalue);
                if (Change.changed)
                    State = White;
                break;
            case Fading:
                break;
            case Off:
                State_Off();
                break;
            default:
                break;
            }
        }

    }
    else
        State_Off();

    //debugging
    Serial.println(State);
    Serial.println(fadingflag);
    Serial.println();


}//end of loop

//-------------------------------------------------------------------------------------------------------
