#define mtr_dir         PORTBbits.RB7
#define mtr_esq         PORTBbits.RB6
#define velocidade_max  600

//função para mover motores, recebe como parâmetros o lado (d/e), sentido (f/t) e velocidade (em %)
void moverMotor(char lado, char sentido, unsigned int velocidade){
    velocidade *= velocidade_max/100;
    if(lado == 'e'){
        mtr_esq = 1; 
        if(sentido == 'f'){
            PDC0L = velocidade & 0b11111111;
            PDC0H = (velocidade >> 8) & 0b11111111;
            PDC1L = 0;
            PDC1H = 0;
        }
        else if(sentido == 't'){
            PDC1L = velocidade & 0b11111111;
            PDC1H = (velocidade >> 8) & 0b11111111;
            PDC0L = 0;
            PDC0H = 0;
        }     
    }
    else if(lado == 'd'){
        mtr_dir = 1; 
        if(sentido == 'f'){
            PDC2L = velocidade & 0b11111111;
            PDC2H = (velocidade >> 8) & 0b11111111;
            PDC3L = 0;
            PDC3H = 0;
        }
        else if(sentido == 't'){
            PDC3L = velocidade & 0b11111111;
            PDC3H = (velocidade >> 8) & 0b11111111;
            PDC2L = 0;
            PDC2H = 0;
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