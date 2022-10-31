#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define dim_testo_in 0
#define dim_riga_comando 30
#define linea_max 1025

struct Line{
    int size;
    int currversion;
    int num_version;
    char ** lines;
};

typedef struct Line line;



void aggiorna_riga(line * linea, char * riga){
    int dim=strlen(riga);
    linea->currversion++;
    linea->num_version++;
    if(linea->currversion!=linea->num_version){
        linea->num_version--;
        for(;linea->num_version>linea->currversion;linea->num_version--){
            free(linea->lines[linea->num_version-1]);
        }
    }
    linea->num_version=linea->currversion;
    if(linea->num_version>=linea->size) {
        linea->size+=100;
        linea->lines = (char **) realloc(linea->lines, (linea->size) * sizeof(char *));
    }
    linea->lines[linea->currversion-1] =  malloc((dim+1) * sizeof(char));
    strcpy((linea->lines)[linea->currversion-1],riga);
    return;
}

void alloca_spazio_righe(line *** testo_ptr, int i2, int num_righe){
    line ** testo=*testo_ptr;
    int riga;
    testo = (line **) realloc(testo, (i2)*sizeof(line *));
    for(riga=num_righe;riga<i2;riga++){
        testo[riga] = (line *) malloc(sizeof(line));
        testo[riga]->currversion = 0;
        testo[riga]->num_version = 0;
        testo[riga]->size=0;
        testo[riga]->lines=NULL;
    }
    *testo_ptr=testo;
    return;
}

void change (line *** testo_ptr, int i1, int i2, int * num_righe, int * last_riga){
    line ** testo = *testo_ptr;
    int riga;
    char buffer[linea_max];
    if(i2> *last_riga)
        *last_riga=i2;
    if(i2> * num_righe) {
        alloca_spazio_righe(&testo,i2,*num_righe);
        * num_righe = i2;
    }
    for (riga=i1-1;riga<=i2-1;riga++){
        fgets(buffer,linea_max,stdin);
        aggiorna_riga(testo[riga],buffer);
    }
    *testo_ptr=testo;
    return;
}


void salva_indici_modificati_change(int i1, int i2,int last_riga, int * last_indice_modificato,int ** elenco_indici_modificati_ptr){
    int curr_indice_modificato=*last_indice_modificato;
    int * elenco_indici_modificati = *(elenco_indici_modificati_ptr);
    *(last_indice_modificato)=*(last_indice_modificato)+4;
    elenco_indici_modificati=(int *) realloc(elenco_indici_modificati, (*last_indice_modificato) * sizeof(int));
    elenco_indici_modificati[curr_indice_modificato - 1] = i1;      //PRIMO INDICE DA RIPRISTINARE CON L'UNDO
    elenco_indici_modificati[curr_indice_modificato ] = i2;         //ULTIMO INDICE DA RIPRISTINARE CON L'UNDO
    elenco_indici_modificati[curr_indice_modificato + 1] = last_riga; //LAST RIGA DA RIPRISTINARE CON L'UNDO
    if(i2>last_riga)                                                   //LAST RIGA DA RIPRISTINARE CON REDO
        elenco_indici_modificati[curr_indice_modificato +2] = i2;
    else
        elenco_indici_modificati[curr_indice_modificato +2] = last_riga;
    *elenco_indici_modificati_ptr=elenco_indici_modificati;
    return;
}        //la change nuova dovrà solamente leggere gli indici già preparati da questa funzione e aggiornare con la solita aggiorna riga


void salva_indici_modificati_del(int i1, int i2,int last_riga, int * last_indice_modificato,int ** elenco_indici_modificati_ptr){
    int curr_indice_modificato=*last_indice_modificato;
    int * elenco_indici_modificati = *(elenco_indici_modificati_ptr);
    *last_indice_modificato+=4;
    elenco_indici_modificati=(int *) realloc(elenco_indici_modificati, (*last_indice_modificato) * sizeof(int));
    if(i1==0||i1>last_riga){
        elenco_indici_modificati[curr_indice_modificato - 1] = -1;
        elenco_indici_modificati[curr_indice_modificato ] = -1;
        elenco_indici_modificati[curr_indice_modificato + 1] = -1;
        elenco_indici_modificati[curr_indice_modificato +2] = -1;
    }
    else{
        elenco_indici_modificati[curr_indice_modificato - 1] = i1;        //PRIMO INDICE DA RIPRISTINARE CON L'UNDO
        if(i2>last_riga)
            elenco_indici_modificati[curr_indice_modificato ] = i1-1;    //ULTIMO INDICE DA RIPRISTINARE CON L'UNDO
        else
            elenco_indici_modificati[curr_indice_modificato ] = i1+last_riga-i2-1;    //ULTIMO INDICE DA RIPRISTINARE CON L'UNDO
        elenco_indici_modificati[curr_indice_modificato + 1] = last_riga;   //LAST RIGA DA RIPRISTINARE CON L'UNDO
        if(i2>last_riga)
            elenco_indici_modificati[curr_indice_modificato +2] = i1-1;         //LAST RIGA DA RIPRISTINARE CON REDO
        else
            elenco_indici_modificati[curr_indice_modificato +2] = last_riga - (i2-i1+1);
    }
    *elenco_indici_modificati_ptr=elenco_indici_modificati;
    return;
}

void delete(line *** testo_ptr, int i1, int i2, int * last_riga){
    int gap=*last_riga-(i2-i1+1);
    line ** testo = *testo_ptr;
    int riga;
    if(i1==0)
        return;
    if(i1>*last_riga)
        return;
    if(i2>*last_riga) {
        *last_riga=i1-1;
        return;
    }
    else{
        for(riga=i2+1;riga<=(*last_riga);riga++){
            aggiorna_riga(testo[i1-1],testo[riga-1]->lines[testo[riga-1]->currversion-1]);
            i1++;
        }
        *last_riga=gap;
    }
    *testo_ptr=testo;
    return;
}

void stampa(line ** testo, int i1, int i2,int last_riga){
    int i;
    if(i1==0&&i2==0)
        fputs(".\n",stdout);
    else{
        for(i=i1;i<=i2;i++){
            if(i>last_riga)
                fputs(".\n",stdout);
            else
                fputs( testo[i-1]->lines[testo[i-1]->currversion - 1],stdout);
        }
    }
    return;
}

void undo(line *** testo_ptr,int * elenco_indici_modificati,int stato_comando_corrente,int * last_riga){
    line ** testo = *testo_ptr;
    int i1,i2,riga;
    i1=elenco_indici_modificati[4*stato_comando_corrente];
    if(i1==-1)
        return;
    i2=elenco_indici_modificati[4*stato_comando_corrente+1];
    *last_riga=elenco_indici_modificati[4*stato_comando_corrente+2];
    for(riga=i1;riga<=i2;riga++){
        testo[riga-1]->currversion--;
    }
    *testo_ptr=testo;
    return;
}

void redo(line *** testo_ptr,int * elenco_indici_modificati,int stato_comando_corrente,int * last_riga){
    line ** testo = *testo_ptr;
    int i1,i2,riga;
    i1=elenco_indici_modificati[4*stato_comando_corrente];
    if(i1==-1)
        return;
    i2=elenco_indici_modificati[4*stato_comando_corrente+1];
    *last_riga=elenco_indici_modificati[4*stato_comando_corrente+3];
    for(riga=i1;riga<=i2;riga++){
        testo[riga-1]->currversion++;
    }
    *testo_ptr=testo;
    return;
}


int main() {
    int i,ind_riga_reale,righe_cancellate=0,last_indice_modificato=1,last_riga=-1;
    int * elenco_indici_modificati=NULL,indicatore_comando=0,ultimo_comando=0,stato_comando_corrente=0;
    char testo_comando[dim_riga_comando];
    line ** testo;
    testo = (line **) malloc(dim_testo_in*sizeof(line *));
    elenco_indici_modificati = (int *) malloc (dim_testo_in*sizeof(int));
    int length,i1,i2,num_righe=dim_testo_in;
    while(1){
        fgets(testo_comando,dim_riga_comando,stdin);
        length=strlen(testo_comando);
        if(testo_comando[0]==113)                 //QUIT
            return 0;
        switch(testo_comando[length-2]) {
            case 99 :           //CHANGE
                if(indicatore_comando>stato_comando_corrente){
                    //DEVO FARE DELLE REDO
                    for(;stato_comando_corrente<=indicatore_comando-1;stato_comando_corrente++){
                        redo(&testo,elenco_indici_modificati,stato_comando_corrente,&last_riga);
                    }
                    stato_comando_corrente=indicatore_comando;
                    last_indice_modificato=indicatore_comando*4+1;
                }
                else if(indicatore_comando<stato_comando_corrente){
                    //DEVO FARE DELLE UNDO
                    stato_comando_corrente--;
                    for(;stato_comando_corrente>=indicatore_comando;stato_comando_corrente--){
                        undo(&testo,elenco_indici_modificati,stato_comando_corrente,&last_riga);
                    }
                    stato_comando_corrente=indicatore_comando;
                    last_indice_modificato=indicatore_comando*4+1;
                }
                indicatore_comando++;ultimo_comando=indicatore_comando;stato_comando_corrente=indicatore_comando;
                sscanf(testo_comando,"%d,%dc",&i1,&i2);
                salva_indici_modificati_change(i1,i2,last_riga,&last_indice_modificato,&elenco_indici_modificati);
                change(&testo,i1,i2,&num_righe,&last_riga);
                //elenco_indici_modificati[last_indice_modificato-5]=last_riga;
                break;
            case 'd' :           //DELETE
                if(indicatore_comando>stato_comando_corrente){
                    //DEVO FARE DELLE REDO
                    for(;stato_comando_corrente<=indicatore_comando-1;stato_comando_corrente++){
                        redo(&testo,elenco_indici_modificati,stato_comando_corrente,&last_riga);
                    }
                    stato_comando_corrente=indicatore_comando;
                    last_indice_modificato=indicatore_comando*4+1;
                }
                else if(indicatore_comando<stato_comando_corrente){
                    //DEVO FARE DELLE UNDO
                    stato_comando_corrente--;
                    for(;stato_comando_corrente>=indicatore_comando;stato_comando_corrente--){
                        undo(&testo,elenco_indici_modificati,stato_comando_corrente,&last_riga);
                    }
                    stato_comando_corrente=indicatore_comando;
                    last_indice_modificato=indicatore_comando*4+1;
                }
                indicatore_comando++;ultimo_comando=indicatore_comando;stato_comando_corrente=indicatore_comando;
                sscanf(testo_comando,"%d,%dd",&i1,&i2);
                salva_indici_modificati_del(i1,i2,last_riga,&last_indice_modificato,&elenco_indici_modificati);
                delete(&testo,i1,i2,&last_riga);
                break;
            case 112 :           //PRINT
                if(indicatore_comando>stato_comando_corrente){
                    //DEVO FARE DELLE REDO
                    for(;stato_comando_corrente<=indicatore_comando-1;stato_comando_corrente++){
                        redo(&testo,elenco_indici_modificati,stato_comando_corrente,&last_riga);
                    }
                    stato_comando_corrente=indicatore_comando;
                    last_indice_modificato=indicatore_comando*4+1;
                }
                else if(indicatore_comando<stato_comando_corrente){
                    //DEVO FARE DELLE UNDO
                    stato_comando_corrente--;
                    for(;stato_comando_corrente>=indicatore_comando;stato_comando_corrente--){
                        undo(&testo,elenco_indici_modificati,stato_comando_corrente,&last_riga);
                    }
                    stato_comando_corrente=indicatore_comando;
                    last_indice_modificato=indicatore_comando*4+1;
                }
                else{
                    //NON DEVO FARE NULLA
                }
                sscanf(testo_comando,"%d,%dp",&i1,&i2);
                stampa(testo,i1,i2,last_riga);
                break;
            case 'u' :           //UNDO
                sscanf(testo_comando,"%du",&i1);
                indicatore_comando-=i1;
                if(indicatore_comando<0)
                    indicatore_comando=0;
                break;
            case 'r':           //REDO

                sscanf(testo_comando,"%dr",&i1);
                indicatore_comando+=i1;
                if(indicatore_comando>ultimo_comando)
                    indicatore_comando=ultimo_comando;
                break;
            case '.':
                break;
        }
    }
}
