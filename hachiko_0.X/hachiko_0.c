#include <xc.h>
#include <pic18f4431.h>
#include "config.h"
#include "bt.h"
#include "pwm.h"
#include "timer.h"

#define _XTAL_FREQ      20000000        //mudar para 20 MHz quando for testar na placa
#define dist_dir        PORTAbits.RA0   //RA0
#define dist_fdir       PORTAbits.RA3   //RA1
#define dist_cent       PORTCbits.RC0   //RC0   
#define dist_fesq       PORTAbits.RA2   //RA2
#define dist_esq        PORTAbits.RA4   //RA4
#define lin_fdir        PORTCbits.RC3
#define lin_tdir        PORTCbits.RC5
#define lin_tesq        PORTEbits.RE1
#define lin_fesq        PORTCbits.RC4
#define mtr_dir         PORTBbits.RB7
#define mtr_esq         PORTBbits.RB6
#define led0            PORTDbits.RD0
#define led1            PORTDbits.RD1
#define velocidade_max  600             //mudar para 600 quando for testar na placa

char lado, sentido;
char *acao;
unsigned int velocidade;
int ini = 0;

void moverMotor(char lado, char sentido, unsigned int velocidade);
void pararMotor();
void testarDistancia();
void testarLinha();
void estrela();
void iniciar();
void sensorUSART();
void pwmUSART();
void arco(unsigned int velocidade_dir, unsigned int velocidade_esq);
void dibre();
void testarTMR(unsigned int b);

void __interrupt() ISR(void){
    if(PIR1bits.RCIF){
        PIR1bits.RCIF = 0x00;
        if(RCSTAbits.FERR || RCSTAbits.OERR){       //testa se houve erro de frame ou overrun       
            RCSTAbits.CREN = 0x00;                  
            RCSTAbits.CREN = 0x01;
        }
        
        if(readUSART() == 'L')  acao = "linha";
        
        if(readUSART() == 'D')  acao = "distancia";
        
        if(readUSART() == 'T')  acao = "toquinho";
        
        if(readUSART() == 'A')  acao = "arco";
        
        if(readUSART() == 'E')  acao = "estrela";
        
        if(readUSART() == 'Z')  acao = "zigzag";
        
        if(readUSART() == 'R')  acao = "teste";
        
        if(readUSART() == 'X')  acao = "gd";
        
        if(readUSART() == 'Y')  acao = "ge";
        
        if(readUSART() == 'd')  acao = "direito";
        
        if(readUSART() == 'e')  acao = "esquerdo";
        
        if(readUSART() == 'f')  acao = "frente";
        
        if(readUSART() == 't')  acao = "tras";
        
        if(readUSART() == 'm')  acao = "motor";
        
        if(readUSART() == '+'){
            velocidade += 10*velocidade_max/100;
            if(velocidade >= 600)   velocidade = 10*velocidade_max/100;
        }
        
        if(readUSART() == '-'){
            velocidade -= 10*velocidade_max/100;
            if(velocidade <= 59)    velocidade = 50*velocidade_max/100; //senão rolar, usar 50%
        }
        
        if(readUSART() == 'G')  ini = 1;
        
        if(readUSART() == 'P'){
            pararMotor();
            acao = "parar";
        }
    }
}

void main(void) {

    setIO();
    setBT();
    setPWM();
    setTMR0();
    setTMR1();
    
    ADCHS = 0b01000010;         //habilita os pinos AN7 e AN8(RE1 e RE2, respectivamente) como canais de conversao analogica digital
    
    
    INTCONbits.GIE = 1;         //habilita interrupção global
    INTCONbits.PEIE = 1;        //habilita interrupção periférica
       
    led0 = 0;
    led1 = 1;
    
    //TMR0L = 0x3D;
    //TMR0H = 0xB4;             //começa tmr0 em aproximadamente 12,1072 ms
    
    while(ini == 0);
    stringUSART("aqui");
    
    while(1){
        if(acao == "linha"){
            while(1){
                testarLinha();
                
                if(acao == "parar"){
                    pararMotor();
                    break;
                }
            }
        }
        
        if(acao == "distancia"){
            while(1){
                testarDistancia();
                
                if(acao == "parar"){
                    pararMotor();
                    break;
                }
            }
        }
        
        if(acao == "arco"){
            TMR0ON = 1;
            TMR0L = 0x78;               //base de tempo de 100 ms modo 16 bit
            TMR0H = 0xF8;
            char cont_tmr = 0x00;
            while(cont_tmr < 0x64){
                if(dist_esq && dist_fesq && dist_cent && dist_fdir && dist_dir){
                    moverMotor('d','f',75); //mudar para 75 porque deu top
                    moverMotor('e','f',90);
                }
                else if(!dist_esq && dist_fesq && dist_cent && dist_fdir && dist_dir){       //apenas sensor esquerdo
                    moverMotor('e','t',60);
                    moverMotor('d','f',60);
                }
                else if(dist_esq && dist_fesq && !dist_cent && dist_fdir && dist_dir){  //apenas sensor do centro
                    moverMotor('e','f',60);                                              //lembrar de mudar para 100 (0 apenas teste)
                    moverMotor('d','f',60);
                    //pararMotor();
                }
                else if(dist_esq && !dist_fesq && dist_cent && !dist_fdir && dist_dir){ //os frontais das pontas
                    moverMotor('e','f',60);                                              //lembrar de mudar para 100 (0 apenas teste)
                    moverMotor('d','f',60);
                    //pararMotor();
                }
                else if(dist_esq && dist_fesq && dist_cent && !dist_fdir && dist_dir){  //apenas frontal direito
                    moverMotor('e','f',60);
                    moverMotor('d','t',60);
                }
                else if(dist_esq && dist_fesq && !dist_cent && !dist_fdir && dist_dir){ //frontal direito e central
                    moverMotor('e','f',60);
                    moverMotor('d','t',60);
                }
                else if(dist_esq && !dist_fesq && dist_cent && dist_fdir && dist_dir){  //apenas frontal esquerdo
                    moverMotor('e','t',60);
                    moverMotor('d','f',60);
                }
                else if(dist_esq && !dist_fesq && !dist_cent && dist_fdir && dist_dir){ //frontal esquerdo e central
                    moverMotor('e','t',60);
                    moverMotor('d','f',60);
                }
                else if(dist_esq && dist_fesq && dist_cent && dist_fdir && !dist_dir){  //apenas direito
                    moverMotor('e','f',60);
                    moverMotor('d','t',60);
                }
                else if(dist_esq && !dist_fesq && !dist_cent && !dist_fdir && dist_dir){
                    moverMotor('d','f',60);
                    moverMotor('e','f',60);
                }
                else{
                    moverMotor('d','f',60); //mudar para 75 porque deu top
                    moverMotor('e','f',90);
                }
                
                if(TMR0IF){
                    TMR0IF  = 0;
                    TMR0L   = 0x78;
                    TMR0H   = 0xF8;
                    cont_tmr++;
                }
                
                if(acao == "parar"){
                    pararMotor();
                    break;
                }
            }
            TMR0ON = 0;
        }
        
        if(acao == "toquinho"){
            //TMR0ON = 1;     //liga o timer0
            //TMR0L = 0x78;   //inicializa o timer para base de tempo 100 ms
            //TMR0H = 0xF8;
            TMR1ON  = 1;        //liga o tmr1
            TMR1L   = 0xDE;     //inicializa o timer para base de tempo 100 ms
            TMR1H   = 0x0B;
            char aux = 0x00;
            int cont = 0;
            while(aux < 0x0A){              //enquanto aux for menor que 10s 
                if(TMR1IF){
                    TMR1IF = 0;
                    TMR1L = 0xDE;           //inicializa o timer para base de tempo 100 ms 
                    TMR1H = 0x0B;
                    cont++;
                }
                if(cont < 10){
                    if(!dist_esq && dist_fesq && dist_cent && dist_fdir && dist_dir){       //apenas sensor esquerdo
                        moverMotor('e','t',60);
                        moverMotor('d','f',60);
                    }
                    else if(dist_esq && dist_fesq && !dist_cent && dist_fdir && dist_dir){  //apenas sensor do centro
                        moverMotor('e','f',60);                                              //lembrar de mudar para 100 (0 apenas teste)
                        moverMotor('d','f',60);
                        //pararMotor();
                    }
                    else if(dist_esq && !dist_fesq && dist_cent && !dist_fdir && dist_dir){ //os frontais das pontas
                        moverMotor('e','f',60);                                              //lembrar de mudar para 100 (0 apenas teste)
                        moverMotor('d','f',60);
                        //pararMotor();
                    }
                    else if(dist_esq && dist_fesq && dist_cent && !dist_fdir && dist_dir){  //apenas frontal direito
                        moverMotor('e','f',60);
                        moverMotor('d','t',60);
                    }
                    else if(dist_esq && dist_fesq && !dist_cent && !dist_fdir && dist_dir){ //frontal direito e central
                        moverMotor('e','f',60);
                        moverMotor('d','t',60);
                    }
                    else if(dist_esq && !dist_fesq && dist_cent && dist_fdir && dist_dir){  //apenas frontal esquerdo
                        moverMotor('e','t',60);
                        moverMotor('d','f',60);
                    }
                    else if(dist_esq && !dist_fesq && !dist_cent && dist_fdir && dist_dir){ //frontal esquerdo e central
                        moverMotor('e','t',60);
                        moverMotor('d','f',60);
                    }
                    else if(dist_esq && dist_fesq && dist_cent && dist_fdir && !dist_dir){  //apenas direito
                        moverMotor('e','f',60);
                        moverMotor('d','t',60);
                    }
                    else if(dist_esq && !dist_fesq && !dist_cent && !dist_fdir && dist_dir){
                        moverMotor('d','f',60);
                        moverMotor('e','f',60);
                    }
                    else{
                        if(cont < 1){                     //se a base for 0 ou a cada 1 s 
                            moverMotor('d','f',60);
                            moverMotor('e','f',60);
                        }
                        else{                               //se a base for entre 0 e 1s
                            pararMotor();
                        } 
                    }
                }
                else{
                    cont = 0;
                    aux++;
                    picUSART(aux + 48);
                }
                            
                if(acao == "parar"){
                    pararMotor();
                    break;
                }
            }
            TMR1ON = 0;
        }
        
        if(acao == "estrela"){
            while(1){
                estrela();
                
                if(acao == "parar"){
                    pararMotor();
                    break;
                }
            }
        }
        
        if(acao == "zigzag"){
            /* inicializar uma variável de contagem de tempo
             * inicializar uma variável de contagem de zigzag
             * quando a variável for zero, giro pra esquerda/direita (70 ms)
             * inicializa timer com base de 10 ms
             * enquanto o contador de zigzag for menor que 12
             * enquanto o tempo for menor que 250 ms, ficar testando os sensores e, senão ver nada, ir pra frente
             * parar o motor por 10 ms
             * enquanto o tempo for menor que 70 ms, ficar testando os sensores e, senão ver nada, girar pra direita/esquerda
             * parar o motor por 10 ms
             * aumenta contador de zigzag
             */
            char cont_zig = 0x00;
            char cont_tmr = 0x00;
            moverMotor('d','f',60);     //gira pra esquerda...
            moverMotor('e','t',60);
            __delay_ms(70);             //por 50 ms
            /*TMR0ON = 1;
            TMR0L = 0x3F;               //base de tempo de 10 ms modo 16 bit
            TMR0H = 0xFF;*/
            TMR1ON = 1;
            TMR1L = 0xDE;
            TMR1H = 0x0B;
            while(cont_zig < 0x0C){     //enquanto o contador do zigzag for menor que 12
                while(cont_tmr < 0x19){ //enquanto o contador de timer for menor 250 ms
                    if(TMR1IF){         //testa se estourou a base de tempo 10 ms
                        /*TMR0IF  = 0;
                        TMR0L   = 0x3F;
                        TMR0H   = 0xFF;*/
                        TMR1IF = 0;
                        TMR1L = 0xDE;
                        TMR1H = 0x0B;
                        cont_tmr++;
                    }
                    if(!dist_esq && dist_fesq && dist_cent && dist_fdir && dist_dir){           //apenas sensor esquerdo
                        moverMotor('e','t',60);
                        moverMotor('d','f',60);
                    }
                    else if(dist_esq && dist_fesq && !dist_cent && dist_fdir && dist_dir){      //apenas sensor do centro
                        moverMotor('e','f',60);                                                 //lembrar de mudar para 100 (0 apenas teste)
                        moverMotor('d','f',60);
                        //pararMotor();
                    }
                    else if(dist_esq && !dist_fesq && dist_cent && !dist_fdir && dist_dir){     //os frontais das pontas
                        moverMotor('e','f',60);                                                 //lembrar de mudar para 100 (0 apenas teste)
                        moverMotor('d','f',60);
                        //pararMotor();
                    }
                    else if(dist_esq && dist_fesq && dist_cent && !dist_fdir && dist_dir){      //apenas frontal direito
                        moverMotor('e','f',60);
                        moverMotor('d','t',60);
                    }
                    else if(dist_esq && dist_fesq && !dist_cent && !dist_fdir && dist_dir){     //frontal direito e central
                        moverMotor('e','f',60);
                        moverMotor('d','t',60);
                    }
                    else if(dist_esq && !dist_fesq && dist_cent && dist_fdir && dist_dir){      //apenas frontal esquerdo
                        moverMotor('e','t',60);
                        moverMotor('d','f',60);
                    }
                    else if(dist_esq && !dist_fesq && !dist_cent && dist_fdir && dist_dir){     //frontal esquerdo e central
                        moverMotor('e','t',60);
                        moverMotor('d','f',60);
                    }
                    else if(dist_esq && dist_fesq && dist_cent && dist_fdir && !dist_dir){      //apenas direito
                        moverMotor('e','f',60);
                        moverMotor('d','t',60);
                    }
                    else if(dist_esq && !dist_fesq && !dist_cent && !dist_fdir && dist_dir){    //os três frontais
                        moverMotor('d','f',60);
                        moverMotor('e','f',60);
                    }
                    else{
                        moverMotor('d','f',60);                                                 //ir pra frente, mudar para 80 depois
                        moverMotor('e','f',60);
                    }
                }
                cont_tmr = 0x00;
                pararMotor();
                __delay_ms(10);
                while(cont_tmr < 0x07){
                    if(TMR1IF){
                        /*TMR0IF  = 0;
                        TMR0L   = 0x3F;
                        TMR0H   = 0xFF;*/
                        TMR1IF = 0;
                        TMR1L = 0xDE;
                        TMR1H = 0x0B;
                        cont_tmr++;
                    }
                    if(!dist_esq && dist_fesq && dist_cent && dist_fdir && dist_dir){           //apenas sensor esquerdo
                        moverMotor('e','t',60);
                        moverMotor('d','f',60);
                    }
                    else if(dist_esq && dist_fesq && !dist_cent && dist_fdir && dist_dir){      //apenas sensor do centro
                        moverMotor('e','f',60);                                                 //lembrar de mudar para 100 (0 apenas teste)
                        moverMotor('d','f',60);
                        //pararMotor();
                    }
                    else if(dist_esq && !dist_fesq && dist_cent && !dist_fdir && dist_dir){     //os frontais das pontas
                        moverMotor('e','f',60);                                                 //lembrar de mudar para 100 (0 apenas teste)
                        moverMotor('d','f',60);
                        //pararMotor();
                    }
                    else if(dist_esq && dist_fesq && dist_cent && !dist_fdir && dist_dir){      //apenas frontal direito
                        moverMotor('e','f',60);
                        moverMotor('d','t',60);
                    }
                    else if(dist_esq && dist_fesq && !dist_cent && !dist_fdir && dist_dir){     //frontal direito e central
                        moverMotor('e','f',60);
                        moverMotor('d','t',60);
                    }
                    else if(dist_esq && !dist_fesq && dist_cent && dist_fdir && dist_dir){      //apenas frontal esquerdo
                        moverMotor('e','t',60);
                        moverMotor('d','f',60);
                    }
                    else if(dist_esq && !dist_fesq && !dist_cent && dist_fdir && dist_dir){     //frontal esquerdo e central
                        moverMotor('e','t',60);
                        moverMotor('d','f',60);
                    }
                    else if(dist_esq && dist_fesq && dist_cent && dist_fdir && !dist_dir){      //apenas direito
                        moverMotor('e','f',60);
                        moverMotor('d','t',60);
                    }
                    else{
                        moverMotor('d','t',60);                                                 //girar pra direita...
                        moverMotor('e','f',60);
                    }
                }
                pararMotor();
                __delay_ms(10);
                
                if(acao == "parar"){
                    pararMotor();
                    break;
                }
                cont_zig++;
            }
            TMR1ON = 0;
            acao = " ";
        }
        
        
        if(acao == "esquerdo"){
            velocidade = 10*velocidade_max/100;
            mtr_dir = 1;
            while(1){
                if(acao == "tras"){
                    PDC1L = velocidade & 0xFF;
                    PDC1H = (velocidade >> 8) & 0xFF;
                    PDC2L = 0x00;
                    PDC2H = 0x00;
                }
                else if(acao == "frente"){
                    PDC1L = 0x00;
                    PDC1H = 0x00;
                    PDC2L = velocidade & 0xFF;
                    PDC2H = (velocidade >> 8) & 0xFF;
                }
                if(acao == "parar")   break;
            }
        }
        
        if(acao == "direito"){
            velocidade = 10*velocidade_max/100;
            mtr_esq = 1;
            while(1){
                if(acao == "frente"){
                    PDC0L = velocidade & 0xFF;
                    PDC0H = (velocidade >> 8) & 0xFF;
                    PDC3L = 0x00;
                    PDC3H = 0x00;
                }
                else if(acao == "tras"){
                    PDC0L = 0x00;
                    PDC0H = 0x00;
                    PDC3L = velocidade & 0xFF;
                    PDC3H = (velocidade >> 8) & 0xFF;
                }
                if(acao == "parar")   break;
            }
        }
        
        if(acao == "motor"){
            velocidade = 10*velocidade_max/100;
            mtr_esq = 1;
            mtr_dir = 1;
            while(1){
                if(acao == "frente"){
                    PDC0L = velocidade & 0xFF;              //btn: 944 mV; conector: 2,61V
                    PDC0H = (velocidade >> 8) & 0xFF;
                    PDC3L = 0x00;
                    PDC3H = 0x00;
                    PDC2L = velocidade & 0xFF;              //btn: 953 mV; conector: 3V
                    PDC2H = (velocidade >> 8) & 0xFF;
                    PDC1L = 0x00;
                    PDC1H = 0x00;
                }
                else if(acao == "tras"){
                    PDC0L = 0x00;
                    PDC0H = 0x00;
                    PDC3L = velocidade & 0xFF;              //btn: 948 mV; conector: 2,77V
                    PDC3H = (velocidade >> 8) & 0xFF;
                    PDC2L = 0x00;
                    PDC2H = 0x00;
                    PDC1L = velocidade & 0xFF;              //btn: 950 mV; conector: 2,8V
                    PDC1H = (velocidade >> 8) & 0xFF;
                }
                if(acao == "parar"){
                    pararMotor();
                    break;
                }
            }
        }
        
        if(acao == "teste"){
            //velocidade = 10*velocidade_max/100;
            mtr_dir = 1;
            mtr_esq = 1;
            /*while(1){
                /*PDC0L = 0x00;
                PDC0H = 0x00;
                PDC1L = velocidade & 0xFF;
                PDC1H = (velocidade >> 8) & 0xFF;
                PDC2L = velocidade & 0xFF;
                PDC2H = (velocidade >> 8) & 0xFF;
                PDC3L = 0x00;
                PDC3H = 0x00;
                
                if(acao == "parar"){
                    pararMotor();
                    break;
                }
            }*/
            moverMotor('d','f',80);     //mudar pra frente
            moverMotor('e','f',80);
            __delay_ms(500);
            //pararMotor();
            moverMotor('d','f',0);
            moverMotor('e','f',0);
            __delay_ms(2000);
            moverMotor('d','t',80);
            moverMotor('e','t',80);
            __delay_ms(500);
            pararMotor();
            acao = ' ';
            //break;
        }   
        
        if(acao == "gd"){
            moverMotor('e','f',60);
            moverMotor('d','t',60);
            __delay_ms(300);
            pararMotor();
            acao = ' ';
            //break;
        }
        
        if(acao == "ge"){
            moverMotor('e','t',60);
            moverMotor('d','f',60);
            __delay_ms(300);
            pararMotor();
            acao = ' ';
            //break;
        }
        
        if(!dist_dir){
            __delay_ms(100);
            picUSART('0');
        }
        else if(!dist_fdir){
            __delay_ms(100);
            picUSART('1');
        }
        else if(!dist_cent){
            __delay_ms(100);
            picUSART('2');
        }
        else if(!dist_fesq){
            __delay_ms(100);
            picUSART('3');
        }
        else if(!dist_esq){
            __delay_ms(100);
            picUSART('4');
        }
        
        /*while(cont < 0x0A){         //enquanto contador for menor 0x0A (10)
            if(TMR1IF){                   //se estourou a flag do tmr0
                TMR1IF  = 0;            //limpa a flag do tmr0
                TMR1L   = 0xDE;
                TMR1H   = 0x0B;         //inicializa base de tempo
                cont++;
                led0 = ~led0;
                led1 = ~led1;
            }
            //testarDistancia();
        }
        pararMotor();
        led0 = ~led0;
        led1 = ~led1;
        stringUSART("acabou");
        while(1);*/
    }
    
    return;
}
//função para mover motores, recebe como parâmetros o lado (d/e), sentido (f/t) e velocidade (em %)
void moverMotor(char lado, char sentido, unsigned int velocidade){
    velocidade *= velocidade_max/100;
    if(lado == 'e'){
        mtr_esq = 1; 
        if(sentido == 'f'){
            PDC2L = velocidade & 0b11111111;
            PDC2H = (velocidade >> 8) & 0b11111111;
            PDC1L = 0;
            PDC1H = 0;
        }
        else if(sentido == 't'){
            PDC1L = velocidade & 0b11111111;
            PDC1H = (velocidade >> 8) & 0b11111111;
            PDC2L = 0;
            PDC2H = 0;
        }     
    }
    else if(lado == 'd'){
        mtr_dir = 1; 
        if(sentido == 'f'){
            PDC0L = velocidade & 0b11111111;
            PDC0H = (velocidade >> 8) & 0b11111111;
            PDC3L = 0;
            PDC3H = 0;
        }
        else if(sentido == 't'){
            PDC3L = velocidade & 0b11111111;
            PDC3H = (velocidade >> 8) & 0b11111111;
            PDC0L = 0;
            PDC0H = 0;
        } 
    }    
}
//função para parar motores
void pararMotor(){
    //mtr_dir = 0;
    //mtr_esq = 0;
    PDC0H = 0x00;
    PDC0L = 0x00;
    PDC1H = 0x00;
    PDC1L = 0x00;
    PDC2H = 0x00;
    PDC2L = 0x00;
    PDC3H = 0x00;
    PDC3L = 0x00;
}
void testarDistancia(){
    if(!dist_esq && dist_fesq && dist_cent && dist_fdir && dist_dir){       //apenas sensor esquerdo
        moverMotor('e','t',80);
        moverMotor('d','f',80);
    }
    else if(dist_esq && dist_fesq && !dist_cent && dist_fdir && dist_dir){  //apenas sensor do centro
        moverMotor('e','f',80);                                             //lembrar de mudar para 100 (0 apenas teste)
        moverMotor('d','f',80);
        //pararMotor();
    }
    else if(dist_esq && !dist_fesq && dist_cent && !dist_fdir && dist_dir){ //os frontais das pontas
        moverMotor('e','f',80);                                             //lembrar de mudar para 100 (0 apenas teste)
        moverMotor('d','f',80);
        //pararMotor();
    }
    else if(dist_esq && !dist_fesq && !dist_cent && !dist_fdir && dist_dir){
        moverMotor('d','f',80);
        moverMotor('e','f',80);
    }
    else if(dist_esq && dist_fesq && dist_cent && !dist_fdir && dist_dir){  //apenas frontal direito
        moverMotor('e','f',80);
        moverMotor('d','t',80);
    }
    else if(dist_esq && dist_fesq && !dist_cent && !dist_fdir && dist_dir){ //frontal direito e central
        moverMotor('e','f',80);
        moverMotor('d','t',80);
    }
    else if(dist_esq && !dist_fesq && dist_cent && dist_fdir && dist_dir){  //apenas frontal esquerdo
        moverMotor('e','t',80);
        moverMotor('d','f',80);
    }
    else if(dist_esq && !dist_fesq && !dist_cent && dist_fdir && dist_dir){ //frontal esquerdo e central
        moverMotor('e','t',80);
        moverMotor('d','f',80);
    }
    else if(dist_esq && dist_fesq && dist_cent && dist_fdir && !dist_dir){  //apenas direito
        moverMotor('e','f',80);
        moverMotor('d','t',80);
    }
    else{
        //moverMotor('d','f',60);
        //moverMotor('e','f',60);
        pararMotor();
    }
}
void testarLinha(){
    if(lin_fdir && lin_fesq){
        moverMotor('d','f',70);     //mudar pra frente
        moverMotor('e','f',70);
        //pararMotor();
    }
    else if(lin_fdir && !lin_fesq){
        moverMotor('d','f',70);     //mudar pra frente
        moverMotor('e','t',70);
        __delay_ms(100);
        //pararMotor();
    }
    else if(!lin_fdir && lin_fesq){
        moverMotor('d','t',70);     //mudar pra trás
        moverMotor('e','f',70);
        __delay_ms(100);
        //pararMotor();
    }
    else if(!lin_fdir && !lin_fesq){
        moverMotor('d','t',70);     //mudar pra trás
        moverMotor('e','t',70);
        __delay_ms(50);
        pararMotor();
        __delay_ms(10);
        moverMotor('d','f',70);     //mudar pra frente
        moverMotor('e','t',70);
        //pararMotor();
    }
}
void iniciar(){
    if(readUSART() == 'G'){
        picUSART('1');
        while(readUSART() == 'G');
        }
    else if(readUSART() == 'S'){
        picUSART('0');
        while(readUSART() == 'S');
    }
}
void testarMotor(){
    moverMotor('d','f',50);
    moverMotor('e','f',50);
    __delay_ms(2000);
    pararMotor();
    __delay_ms(2000);
    moverMotor('d','t',50);
    moverMotor('e','t',50);
    __delay_ms(2000);
    pararMotor();
    __delay_ms(2000);
}
void testarTMR(unsigned int b){
    b = (13 - b)*256/13;
    if(TMR0IF){             //estourou a flag do timer? (51,2 us)
        TMR0IF = 0;         //limpa flag do timer
        TMR0L = b;       //reinicia o valor do TMR0
        //cont++;
         
        //if(cont == cont_aux){
            led0 = ~led0;
            led1 = ~led1;
        //    cont = 0x00;
        //}
    }
}
void estrela(){
    if(lin_fdir && lin_fesq){
        testarDistancia();
    }
    else if(!lin_fdir && lin_fesq){
        moverMotor('d','f',60);
        moverMotor('e','t',60);
    }
    else if(lin_fdir && !lin_fesq){
        moverMotor('d','t',60);
        moverMotor('e','f',60);
    }
    else if(!lin_fdir && !lin_fesq){
        moverMotor('d','t',60);
        moverMotor('e','t',60);
        __delay_ms(50);
        pararMotor();
        __delay_ms(50);
        moverMotor('d','t',60);
        moverMotor('e','f',60);
    }
}
void toquinho(){
    
}
void arco(unsigned int velocidade_dir, unsigned int velocidade_esq){
    /*arco para direita: velocidade_dir > velocidade_esq
     *arco para esquerda: velocidade_esq > velocidade_dir */
    moverMotor('d','f',velocidade_dir);
    moverMotor('e','f',velocidade_esq);
}
void dibre(){ /* da um quarto de volta no dojo e alinha com o centro
               * testar tempo que o chiquinho demoraria pra dar uma volta no 
               * dojo fazendo um arco e considerar 1/4 desse tempo, depois
               * alinhar
               */
    arco(75,90);
    __delay_ms(70);
    moverMotor('e','f',60);
    moverMotor('d','t',60);
    __delay_ms(150);           
}
void pwmUSART(){
    
}