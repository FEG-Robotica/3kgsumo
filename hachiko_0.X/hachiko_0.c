#include <xc.h>
#include <pic18f4431.h>
#include "config.h"
#include "bt.h"
#include "pwm.h"
#include "timer.h"

#define _XTAL_FREQ      20000000        //mudar para 20 MHz quando for testar na placa

// Sensores de distância
#define dist_dir        PORTAbits.RA2   //RA0
#define dist_fdir       PORTAbits.RA3   //RA1
#define dist_cent       PORTCbits.RC0   //RC0   
#define dist_fesq       PORTAbits.RA4   //RA2
#define dist_esq        PORTAbits.RA1   //RA4

// Sensores de linha
#define lin_fdir        PORTCbits.RC3
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
int flag = 1;

void moverMotor(char lado, char sentido, unsigned int velocidade);
void pararMotor();
void testarDistancia();
void testarLinha();
void linha();

void estrela();

void __interrupt() ISR(void){
    if(PIR1bits.RCIF){
        PIR1bits.RCIF = 0x00;
        if(RCSTAbits.FERR || RCSTAbits.OERR){       //testa se houve erro de frame ou overrun       
            RCSTAbits.CREN = 0x00;                  
            RCSTAbits.CREN = 0x01;
        }
        
        if(readUSART() == 'L')  {
            ini = 1;
            acao = "linha";
        }
        
        if(readUSART() == 'D')  {
            ini = 1;
            acao = "distancia";
        }
        
        if(readUSART() == 'T') {
            ini = 1;
            acao = "toquinho";
        }
        
        if(readUSART() == 'A') {
            ini = 1;
            acao = "arco";
        }
        if(readUSART() == 'F') {
            ini = 1;
            acao = "fEstrela";
        }
        
        if(readUSART() == 'E') {
            ini = 1;
            acao = "estrela";
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
    
    INTCONbits.GIE = 1;         //habilita interrupção global
    INTCONbits.PEIE = 1;        //habilita interrupção periférica
       
    led0 = 0;
    led1 = 1;
    
    while(ini == 0){
        // Trava aqui
    }
    stringUSART("Comecou");
    
    __delay_ms(5000);
    
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
                
                if(!lin_fesq || !lin_fdir) {
                    linha();
                } else {
                
                if(dist_esq && dist_fesq && dist_cent && dist_fdir && dist_dir){
                    moverMotor('d','f',70);
                    moverMotor('e','f',80);
                }
                else if(!dist_esq && dist_fesq && dist_cent && dist_fdir && dist_dir){       //apenas sensor esquerdo
                    moverMotor('e','t',70);
                    moverMotor('d','f',70);
                }
                else if(dist_esq && dist_fesq && !dist_cent && dist_fdir && dist_dir){  //apenas sensor do centro
                    moverMotor('e','f',70);                                              //lembrar de mudar para 100 (0 apenas teste)
                    moverMotor('d','f',70);
                    //pararMotor();
                }
                else if(dist_esq && !dist_fesq && dist_cent && !dist_fdir && dist_dir){ //os frontais das pontas
                    moverMotor('e','f',70);                                              //lembrar de mudar para 100 (0 apenas teste)
                    moverMotor('d','f',70);
                    //pararMotor();
                }
                else if(dist_esq && dist_fesq && dist_cent && !dist_fdir && dist_dir){  //apenas frontal direito
                    moverMotor('e','f',70);
                    moverMotor('d','t',70);
                }
                else if(dist_esq && dist_fesq && !dist_cent && !dist_fdir && dist_dir){ //frontal direito e central
                    moverMotor('e','f',70);
                    moverMotor('d','t',70);
                }
                else if(dist_esq && !dist_fesq && dist_cent && dist_fdir && dist_dir){  //apenas frontal esquerdo
                    moverMotor('e','t',70);
                    moverMotor('d','f',70);
                }
                else if(dist_esq && !dist_fesq && !dist_cent && dist_fdir && dist_dir){ //frontal esquerdo e central
                    moverMotor('e','t',70);
                    moverMotor('d','f',70);
                }
                else if(dist_esq && dist_fesq && dist_cent && dist_fdir && !dist_dir){  //apenas direito
                    moverMotor('e','f',70);
                    moverMotor('d','t',70);
                }
                else if(dist_esq && !dist_fesq && !dist_cent && !dist_fdir && dist_dir){
                    moverMotor('d','f',100);
                    moverMotor('e','f',100);
                }
                else{
                    moverMotor('d','f',100);
                    moverMotor('e','f',100);
                }
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
                
                if(!lin_fesq || !lin_fdir) {
                    linha();
                } else {
                if(cont < 10){
                    if(!dist_esq && dist_fesq && dist_cent && dist_fdir && dist_dir){       //apenas sensor esquerdo
                        moverMotor('e','t',80);
                        moverMotor('d','f',80);
                    }
                    else if(dist_esq && dist_fesq && !dist_cent && dist_fdir && dist_dir){  //apenas sensor do centro
                        moverMotor('e','f',100);                                              //lembrar de mudar para 100 (0 apenas teste)
                        moverMotor('d','f',100);
                        //pararMotor();
                    }
                    else if(dist_esq && !dist_fesq && dist_cent && !dist_fdir && dist_dir){ //os frontais das pontas
                        moverMotor('e','f',100);                                              //lembrar de mudar para 100 (0 apenas teste)
                        moverMotor('d','f',100);
                        //pararMotor();
                    }
                    else if(dist_esq && dist_fesq && dist_cent && !dist_fdir && dist_dir){  //apenas frontal direito
                        moverMotor('e','f',70);
                        moverMotor('d','t',70);
                    }
                    else if(dist_esq && dist_fesq && !dist_cent && !dist_fdir && dist_dir){ //frontal direito e central
                        moverMotor('e','f',70);
                        moverMotor('d','t',70);
                    }
                    else if(dist_esq && !dist_fesq && dist_cent && dist_fdir && dist_dir){  //apenas frontal esquerdo
                        moverMotor('e','t',70);
                        moverMotor('d','f',70);
                    }
                    else if(dist_esq && !dist_fesq && !dist_cent && dist_fdir && dist_dir){ //frontal esquerdo e central
                        moverMotor('e','t',70);
                        moverMotor('d','f',70);
                    }
                    else if(dist_esq && dist_fesq && dist_cent && dist_fdir && !dist_dir){  //apenas direito
                        moverMotor('e','f',80);
                        moverMotor('d','t',80);
                    }
                    else if(dist_esq && !dist_fesq && !dist_cent && !dist_fdir && dist_dir){
                        moverMotor('d','f',100);
                        moverMotor('e','f',100);
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
        
        if(acao == "fEstrela"){
            while(1){
                
                if(flag == 1){
                    moverMotor('d','f',100);
                    moverMotor('e','f',100);
                    __delay_ms(30);
                    flag = 0;
                }
                
                estrela();
                
                if(acao == "parar"){
                    pararMotor();
                    break;
                }
            }
        }        
        
        if(!dist_dir){
            __delay_ms(100);
            picUSART('0');
        }
        if(!dist_fdir){
            __delay_ms(100);
            picUSART('1');
        }
        if(!dist_cent){
            __delay_ms(100);
            picUSART('2');
        }
        if(!dist_fesq){
            __delay_ms(100);
            picUSART('3');
        }
        if(!dist_esq){
            __delay_ms(100);
            picUSART('4');
        }
    }
    
    return;
}
//função para mover motores, recebe como parâmetros o lado (d/e), sentido (f/t) e velocidade (em %)
void moverMotor(char lado, char sentido, unsigned int velocidade){
    velocidade *= velocidade_max/100;
    if(lado == 'e'){
        mtr_esq = 1; 
        if(sentido == 'f'){            
            PDC1L = velocidade & 0xFF;
            PDC1H = (velocidade >> 8) & 0xFF;   
            PDC2L = 0x00;
            PDC2H = 0x00;
        }
        else if(sentido == 't'){
            
            PDC1L = 0x00;
            PDC1H = 0x00;
            PDC2L = velocidade & 0xFF;
            PDC2H = (velocidade >> 8) & 0xFF;   
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
    int velTest = 70;
    int velTestParaFrente = 100;                                              //lembrar de mudar para 100 (0 apenas teste)
    if(!dist_esq && dist_fesq && dist_cent && dist_fdir && dist_dir){       //apenas sensor esquerdo
        moverMotor('e','t',velTest);
        moverMotor('d','f',velTest);
    }
    else if(dist_esq && dist_fesq && !dist_cent && dist_fdir && dist_dir){  //apenas sensor do centro
        moverMotor('e','f',velTestParaFrente);                                             
        moverMotor('d','f',velTestParaFrente);
        //pararMotor();
    }
    else if(dist_esq && !dist_fesq && dist_cent && !dist_fdir && dist_dir){ //os frontais das pontas
        moverMotor('e','f',velTestParaFrente);                             
        moverMotor('d','f',velTestParaFrente);
        //pararMotor();
    }
    else if(dist_esq && !dist_fesq && !dist_cent && !dist_fdir && dist_dir){
        moverMotor('d','f',velTestParaFrente);
        moverMotor('e','f',velTestParaFrente);
    }
    else if(dist_esq && dist_fesq && dist_cent && !dist_fdir && dist_dir){  //apenas frontal direito
        moverMotor('e','f',velTest);
        moverMotor('d','t',velTest);
    }
    else if(dist_esq && dist_fesq && !dist_cent && !dist_fdir && dist_dir){ //frontal direito e central
        moverMotor('e','f',velTest);
        moverMotor('d','t',velTest);
    }
    else if(dist_esq && !dist_fesq && dist_cent && dist_fdir && dist_dir){  //apenas frontal esquerdo
        moverMotor('e','t',velTest);
        moverMotor('d','f',velTest);
    }
    else if(dist_esq && !dist_fesq && !dist_cent && dist_fdir && dist_dir){ //frontal esquerdo e central
        moverMotor('e','t',velTest);
        moverMotor('d','f',velTest);
    }
    else if(dist_esq && dist_fesq && dist_cent && dist_fdir && !dist_dir){  //apenas direito
        moverMotor('e','f',velTest);
        moverMotor('d','t',velTest);
    }
    else{
        moverMotor('d','f',100);
        moverMotor('e','f',100);
        //pararMotor();
    }
}

void linha(){
    int velTest = 50;
    if(lin_fdir && !lin_fesq){
        moverMotor('d','f',velTest);     //mudar pra frente
        moverMotor('e','t',velTest);
        __delay_ms(300);
        //pararMotor();
    }
    else if(!lin_fdir && lin_fesq){
        moverMotor('d','t',velTest);     //mudar pra trás
        moverMotor('e','f',velTest);
        __delay_ms(300);
        //pararMotor();
    }
    else if(!lin_fdir && !lin_fesq){
        moverMotor('d','t',velTest);     //mudar pra trás
        moverMotor('e','t',velTest);
        __delay_ms(100);
        pararMotor();
        __delay_ms(10);
        moverMotor('d','f',velTest);     //mudar pra frente
        moverMotor('e','t',velTest);
        __delay_ms(100);
    }
}

void testarLinha(){
    int velTest = 50;
    if(lin_fdir && lin_fesq){
        moverMotor('d','f',velTest);     //mudar pra frente
        moverMotor('e','f',velTest);
    }
    else if(lin_fdir && !lin_fesq){
        moverMotor('d','f',velTest);     //mudar pra frente
        moverMotor('e','t',velTest);
        __delay_ms(300);
    }
    else if(!lin_fdir && lin_fesq){
        moverMotor('d','t',velTest);     //mudar pra trás
        moverMotor('e','f',velTest);
        __delay_ms(300);
    }
    else if(!lin_fdir && !lin_fesq){
        moverMotor('d','t',velTest);     //mudar pra trás
        moverMotor('e','t',velTest);
        __delay_ms(100);
        pararMotor();
        __delay_ms(10);
        moverMotor('d','f',velTest);     //mudar pra frente
        moverMotor('e','t',velTest);
        __delay_ms(100);
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

void estrela(){
    if(lin_fdir && lin_fesq){
        testarDistancia();
    }
    else if(!lin_fdir && lin_fesq){
        moverMotor('d','f',70);
        moverMotor('e','t',70);
        __delay_ms(100);
    }
    else if(lin_fdir && !lin_fesq){
        moverMotor('d','t',70);
        moverMotor('e','f',70);
        __delay_ms(100);
    }
    else if(!lin_fdir && !lin_fesq){
        moverMotor('d','t',80);
        moverMotor('e','t',80);
        __delay_ms(100);
        pararMotor();
        __delay_ms(10);
        moverMotor('d','t',70);
        moverMotor('e','f',70);
        __delay_ms(100);
    }
}