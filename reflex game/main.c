// Zeynep Ozden
// 2018513054

#include <msp430g2553.h>
#include "lcdLib.h"
#include <stdlib.h>
#include <time.h>

#define numberOfRounds 3
#define ledDelayLength 1000
#define roundLength 2000
#define delayBetweenRounds 4000
#define lengthOfEndGameScore 5000
#define lengthOfBestTime 5000
#define lengthStartNew 3000

unsigned volatile int scoreTime = 0;
char volatile gameBool = 0x00;
char volatile gameBool2 = 0x00;
char volatile buttonBool = 0x00;
unsigned short gameCount = 0;
unsigned volatile int player1Time;
unsigned volatile int player2Time;
unsigned volatile int player1Score;
unsigned volatile int player2Score;
unsigned int randomNum;
unsigned volatile int LPM_delay = 0;

void game();
void writeScores();
void delay_ms_LPM(unsigned int);


void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    //Clock settings
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    IFG1 &= ~OFIFG;
    BCSCTL2 |= SELM_1;



    //Timer A control register
    TACTL = TASSEL_2 + MC_1;
    CCR0 =  1000;
    CCTL0 = CCIE;

    //LCD outputs
    P2DIR |= BIT0 + BIT1 + BIT2 + BIT3+ BIT4+ BIT5;

    //LED outputs
    P1DIR |=  BIT6;
    P1DIR |= BIT7;

    //BUTTON CONFIG
    P1DIR &= ~BIT1 + ~BIT2 + ~BIT3 + ~BIT4;
    P1REN |= BIT1 + BIT2 + BIT3 + BIT4;
    P1IE |= BIT1 + BIT2 + BIT3 + BIT4;
    P1IES |= BIT1 + BIT2 + BIT3 + BIT4;
    P1OUT |= BIT1 + BIT2 + BIT3 + BIT4;
    P1OUT &= ~BIT6;
    P1OUT  &= ~BIT7;
    P1IFG &= ~BIT1 + ~BIT2 + ~BIT3 + ~BIT4;

    srand(time(NULL));


    lcdInit();

    while(1) {
        lcdClear();
        lcdSetText("Reflex Game",0,0);
        lcdSetText("Starting new",0,1);

        player1Score=0;
        player2Score=0;
        p1Best = (roundLength+5);
        p2Best = (roundLength+5);
        gameCount=numberOfRounds;

        delay_ms_LPM(lengthStartNew);

        while(gameCount != 0) {
            game();
            delay_ms_LPM(delayBetweenRounds);
        }


        lcdClear();
        if(player1Score>player2Score) {
            lcdSetText("Player 1 WON",0,0);
            lcdSetText("P1:    P2: ",0,1);
            lcdSetInt(player1Score,3,1);
            lcdSetInt(player2Score,10,1);
        }
        else if(player1Score<player2Score) {
            lcdSetText("Player 2 WON",0,0);
            lcdSetText("P1:    P2: ",0,1);
            lcdSetInt(player1Score,3,1);
            lcdSetInt(player2Score,10,1);
        }
        delay_ms_LPM(lengthOfEndGameScore);

    }


}


void game() {
    lcdClear();
    lcdSetText("****************",0,0);
    randomNum=3;
    gameBool |= BIT0;
    _BIS_SR(GIE);

    delay_ms_LPM(ledDelayLength);
    _BIS_SR(GIE);


    //RANDOM LED*********
    randomNum = rand()%2;   //1=green
    if(randomNum==0) {
    //if(1) {
        gameBool &= ~BIT7;//green

        P1OUT |= BIT7;      }
    else {
        gameBool |= BIT7;//red
        P1OUT |= BIT6;
    }



    scoreTime=0;


    while((gameBool & BIT0)==BIT0) {
        _BIS_SR(CPUOFF);
    }


    P1OUT &= ~BIT7;
    P1OUT &= ~BIT6;

    writeScores();

    //Re-set variables
    gameBool = 0x00;
    gameBool2 = 0x00;
    buttonBool = 0x00;
    scoreTime = 0;
    player1Time=0;
    player2Time=0;
}

void writeScores() {   //write scores of ROUND
    lcdClear();
    if( ((gameBool & BIT1)!=0) && ((gameBool & BIT2)!=0)  ) { //P1 correct P2 correct
       if(player2Time>player1Time) {
            lcdSetText("P1 WON:      ms ",0,0);
            lcdSetText("(    ms) faster",0,1);
            lcdSetInt(player1Time,8,0);
            lcdSetInt( abs(player1Time-player2Time) ,1,1);
            player1Score++;
            gameCount--;
        }
        else if(player2Time<player1Time) {
            lcdSetText("P2 WON:      ms ",0,0);
            lcdSetText("(    ms) faster",0,1);
            lcdSetInt(player2Time,8,0);
            lcdSetInt( abs(player2Time-player1Time) ,1,1);
            player2Score++;
            gameCount--;
        }
    }

}

void delay_ms_LPM(unsigned int delayTime) {
    LPM_delay = delayTime;
    _BIS_SR(CPUOFF + GIE);
}

#pragma vector=TIMER0_A0_VECTOR //timer interrupt for ms counting
__interrupt void Timer_A (void)
{

    if(LPM_delay!=0) {
        LPM_delay = LPM_delay - 1;
        if(LPM_delay==0) {
            __bic_SR_register_on_exit(CPUOFF + GIE);
        }
    }
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{

    if((gameBool & BIT0)== BIT0  ) {
        if( ((P1IN & BIT1) == 0) && ((buttonBool & BIT1) == 0) ) {
            if( (gameBool&BIT7) == BIT7  ) {
                gameBool |= BIT3;
                gameBool &= ~BIT1;
                player1Time = 0;
                buttonBool |= BIT2;
            }
            else if((gameBool&BIT3)==0){
                gameBool |= BIT1;
                player1Time = scoreTime;
            }
            buttonBool |= BIT1;
        }

        if( ((P1IN & BIT2) == 0) && ((buttonBool & BIT2) == 0) ) {
            if( (gameBool&BIT7) != BIT7  ) {
                gameBool |= BIT3;
                gameBool &= ~BIT1;
                player1Time = 0;
                buttonBool |= BIT1;
            }
            else if((gameBool&BIT4)==0){
                gameBool |= BIT1;
                player1Time = scoreTime;
            }
            buttonBool |= BIT2;
        }

        if( ((P1IN & BIT3) == 0) && ((buttonBool & BIT3) == 0) ) {
            if( (gameBool&BIT7) == BIT7  ) {
                gameBool |= BIT4;
                gameBool &= ~BIT2;
                player2Time = 0;
                buttonBool |= BIT4;
            }
            else{
                gameBool |= BIT2;
                player2Time = scoreTime;
            }
            buttonBool |= BIT3;
        }

        if( ((P1IN & BIT4) == 0) && ((buttonBool & BIT4) == 0) ) {
            if( (gameBool&BIT7) != BIT7  ) {
                gameBool |= BIT4;
                gameBool &= ~BIT2;
                player2Time = 0;
                buttonBool |= BIT3;
            }
            else{
                gameBool |= BIT2;
                player2Time = scoreTime;
            }
            buttonBool |= BIT4;
        }
    }


    P1IFG = 0x00;
}
