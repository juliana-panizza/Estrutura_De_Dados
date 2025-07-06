
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define SEED    0x12345678

typedef struct {
     uintptr_t * table;
     int size;
     int max;
     uintptr_t deleted;
     char * (*get_key)(void *);
     int metodo; 
     float max_ocupacao; 

}thash;


uint32_t hashf(const char* str, uint32_t h){
    /* One-byte-at-a-time Murmur hash 
    Source: https://github.com/aappleby/smhasher/blob/master/src/Hashes.cpp */
    for (; *str; ++str) {
        h ^= *str;
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }
    return h;
}

// Função para o hash duplo 
uint32_t doubleHash(const char* str)
{
    uint32_t h = 0;
    for(; *str; ++str)
    {
        h = (h * 33) ^(uint32_t)(*str);
    }
    return (h | 1);
}


int hash_insere(thash * h, void * bucket){
    // Verifica taxa de ocupação
    float ocupacao_atual = (float)(h->size + 1) / h->max;
    if(ocupacao_atual >= h->max_ocupacao)
    {
        hash_rehash(h);
    }


    uint32_t hash = hashf(h->get_key(bucket),SEED);
    int pos = hash % (h->max);
    int step = 1;

    if(h->metodo == 1)
    {
        step = doubleHash(h->get_key(bucket)) % h->max;
        if(step == 0)
        {
            step = 1;
        }
    }
    
    while((h->table[pos]) != 0 ){
            if (h->table[pos] == h->deleted)
                break;
            pos = (pos+step) % h->max;
    }    
        
    h->table[pos] = (uintptr_t) bucket;
    h->size += 1;
    return EXIT_SUCCESS;
}

int hash_constroi(thash * h,int nbuckets, char * (*get_key)(void *), int metodo, float max_ocup){
    h->table =calloc(sizeof(void *),nbuckets+1);
    if (h->table == NULL){
        return EXIT_FAILURE;
    }
    h->max = nbuckets+1;
    h->size = 0;
    h->deleted = (uintptr_t)&(h->size);
    h->get_key = get_key;
    h->metodo = metodo;
    h->max_ocupacao = max_ocup;
    return EXIT_SUCCESS;

}


void * hash_busca(thash h, const char * key){
    int pos = hashf(key,SEED) % (h.max);
    int step = 1;

    if(h.metodo == 1)
    {
        step = doubleHash(key) % h.max;
        if(step == 0)
        {
            step =  1;
        }
    }

    while(h.table[pos] != 0){
        if (strcmp(h.get_key((void *)h.table[pos]),key) == 0){
            return (void *)h.table[pos]; 
        }else
            pos = (pos+1)%h.max;
    }
    return NULL;

}


int hash_remove(thash * h, const char * key){
    int pos = hashf(key,SEED) % h->max;
    int step = 1;

    if(h->metodo == 1)
    {
        step = doubleHash(key) % h->max;
        if(step == 0)
        {
            step =  1;
        }
    }

    while(h->table[pos]!=0){
        if (strcmp(h->get_key((void *)h->table[pos]),key) == 0){ 
            free((void *)h->table[pos] );
            h->table[pos] = h->deleted;
            h->size -=1;
            return EXIT_SUCCESS; 
        }else
            pos = (pos+1)%h->max;
    }
    return EXIT_FAILURE;

}

void hash_apaga(thash *h){
    int pos;
    for(pos =0;pos< h->max;pos++){
        if (h->table[pos] != 0){
            if (h->table[pos]!=h->deleted){
                free((void *)h->table[pos]);
            }
        }
    }
    free(h->table);
}


int hash_rehash(thash *h)
{
    int novo_max = (h->max - 1) * 2;
    thash nova;
    hash_constroi(&nova, novo_max, h->get_key, h->metodo, h->max_ocupacao);

    for(int i = 0; i < h->max; i++)
    {
        if(h->table[i] != 0 && h->table[i] != h->deleted)
        {
            void* item = (void *)h->table[i];
            hash_insere(&nova, item); // Realoca na nova tabela
        }
    }

    free(h->table);
    *h = nova;
    return EXIT_SUCCESS;

}

typedef struct{
    char cep[9];
    char cidade[50];
    char estado[3];
}tcep;

char * get_key(void * reg){
   static char chave[6];
   strncpy(chave, ((tcep *)reg)->cep, 5); // Para copiar os cinco primeiros digitos
   chave[5] = '\0';
   return chave;
}


void * aloca_cep(char *cep, char *cidade, char *estado){
    tcep* item = malloc(sizeof(tcep));
    strcpy(item->cep, cep);
    strcpy(item->cidade, cidade);
    strcpy(item->estado, estado);
    return item;
}

void test_hash(int metodo){
    thash h;
    int nbuckets = 10;
    hash_constroi(&h,nbuckets,get_key, metodo, 0.9);

    assert(hash_insere(&h, aloca_cep("01310940", "São Paulo", "SP")) == EXIT_SUCCESS);
    assert(hash_insere(&h, aloca_cep("01311941", "São Paulo", "SP")) == EXIT_SUCCESS);
    assert(hash_insere(&h, aloca_cep("01312942", "São Paulo", "SP")) == EXIT_SUCCESS);
    assert(hash_insere(&h, aloca_cep("04044970", "São Paulo", "SP")) == EXIT_SUCCESS);
    assert(hash_insere(&h, aloca_cep("01010901", "São Paulo", "SP")) == EXIT_SUCCESS);
    assert(hash_insere(&h, aloca_cep("20040002", "Rio de Janeiro", "RJ")) == EXIT_SUCCESS);
    assert(hash_insere(&h, aloca_cep("30110030", "Belo Horizonte", "MG")) == EXIT_SUCCESS);
    assert(hash_insere(&h, aloca_cep("30110031", "Belo Horizonte", "MG")) == EXIT_SUCCESS);
    assert(hash_insere(&h, aloca_cep("30110032", "Belo Horizonte", "MG")) == EXIT_SUCCESS);
    assert(hash_insere(&h, aloca_cep("30110033", "Belo Horizonte", "MG")) == EXIT_SUCCESS);

    hash_apaga(&h);
}

void test_search(int metodo){
    thash h;
    int nbuckets = 10;
    tcep *cep;
    hash_constroi(&h,nbuckets,get_key, metodo, 0.9);

    hash_insere(&h, aloca_cep("01310940", "São Paulo", "SP"));
    hash_insere(&h, aloca_cep("20040002", "Rio de Janeiro", "RJ"));
    hash_insere(&h, aloca_cep("30110030", "Belo Horizonte", "MG"));

    cep = hash_busca(h, "01310");
    assert(strcmp(cep->cidade, "São Paulo") == 0);
    assert(strcmp(cep->estado, "SP") == 0);

    cep = hash_busca(h, "20040");
    assert(strcmp(cep->cidade, "Rio de Janeiro") == 0);
    assert(strcmp(cep->estado, "RJ") == 0);

    cep = hash_busca(h, "30110");
    assert(strcmp(cep->cidade, "Belo Horizonte") == 0);

    cep = hash_busca(h, "99999");
    assert(cep == NULL);

    hash_apaga(&h);
}

void test_remove(int metodo){
    thash h;
    int nbuckets = 10;
    tcep* cep;
    hash_constroi(&h,nbuckets,get_key, metodo, 0.9);

    hash_insere(&h, aloca_cep("01310940", "São Paulo", "SP"));
    hash_insere(&h, aloca_cep("20040002", "Rio de Janeiro", "RJ"));
    hash_insere(&h, aloca_cep("30110030", "Belo Horizonte", "MG"));

    cep = hash_busca(h, "01310");
    assert(strcmp(cep->cidade, "São Paulo") == 0);

    cep = hash_busca(h, "30110");
    assert(strcmp(cep->estado, "MG") == 0);

    assert(h.size == 3);

    assert(hash_remove(&h, "01310") == EXIT_SUCCESS);
    cep = hash_busca(h, "01310");
    assert(cep == NULL);
    assert(h.size == 2);

    assert(hash_remove(&h, "01310") == EXIT_FAILURE);

    cep = hash_busca(h, "30110");
    assert(strcmp(cep->cidade, "Belo Horizonte") == 0);

    hash_apaga(&h);

}

int main(){
    printf("== Teste Hash Simples ==\n");
    test_hash(0);
    test_search(0);
    test_remove(0);
    printf("Hash Simples: SUCCESS!\n\n");

    printf("== Teste Hash Duplo ==\n");
    test_hash(1);
    test_search(1);
    test_remove(1);
    printf("Hash Duplo: SUCCESS!\n\n");

    return 0;
}
