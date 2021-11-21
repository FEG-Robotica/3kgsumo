void setBT(){
    /*For a device with FOSC of 20 MHz, desired baud rate of 9600, Asynchronous mode, 8-bit BRG:
    Desired Baud Rate= FOSC / (16 ([SPBRGH:SPBRG] + 1))
    Solving for SPBRGH:SPBRG:
    X = ((Fosc / Desired Baud Rate)/16) - 1
    = ((20000000 / 9600) / 16) - 1
    = [130.21] = 130 
    Calculated Baud Rate=20000000 / (16 (130 + 1))
    = 9541.98
    Error = (Calculated Baud Rate ? Desired Baud Rate) / Desired Baud Rate
    = (9542  9600) / 9600 = 0.6%*/    
    SPBRG = 0x82;           //130 de baud rate
    
    //configurar USART(8 bits, sem paridade, 1 stop bit, 9600)
    TXSTAbits.TXEN = 1;     //habilita transmiss�o pic-bt
    TXSTAbits.SYNC = 0;     //habilita modo assincrono      
    TXSTAbits.BRGH = 1;     //baud rate high speed
    
    RCSTAbits.SPEN = 1;     //habilita modo serial
    RCSTAbits.CREN = 1;     //habilita recep��o continua
    
    //configurar interrup��es
    PIR1bits.RCIF = 0;      //limpa flag de interrup��o da USART
    
    PIE1bits.RCIE = 1;      //habilita interrup��o da USART
}
void testBuff(){
    while(!TXSTAbits.TRMT);    //aguarda buffer esvaziar     
}
void echo(){            //fun��o para escrever na USART o que foi digitado
    TXREG = RCREG;      //caractere digitado
    testBuff();
    TXREG = 0x0D;
    testBuff();
    TXREG = 0x0A;
    testBuff();
}
void picUSART(char status){
    TXREG = status;
    testBuff();
    TXREG = 0x0D;
    testBuff();
}
char readUSART(){
    char ler = RCREG;
    return ler;
}
void stringUSART(char *frase){
    int i = 0;
    while(frase[i] != '\0'){
        TXREG = frase[i];
        testBuff();
        i++;
    }
    TXREG = 0x0D;
    testBuff();
}
void readString(){
    /*ler uma string enviada pelo bt
     obedecer um padr�o de leitura para selecionar uma estrat�gia inicial e
     uma estrat�gia secund�ria para o rob�
     
     PADR�O:
     n� da estrat�gia - estrat�gia - lado inicial - n�mero de tentativas
     
     */
}