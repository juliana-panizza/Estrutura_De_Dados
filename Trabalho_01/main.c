#include<stdio.h>
#include<stdlib.h>
#include<float.h>
#include<string.h>
#include<assert.h>
#define MAX 128

/*Definições desenvolvedor usuario*/
/*As seguintes funções foram modificadas para comportar um vetor de 128 floats e uma string de 100 caracteres, representando respectivamente o embedding de face e o id da pessoa:*/
typedef struct _reg{
    float embedding[MAX];
    char id_pessoa[100];
}treg;

void * aloca_reg(float vetor_face[MAX], char id_pessoa[]){
    treg * reg;
    reg = malloc(sizeof(treg));
    memcpy(reg->embedding, vetor_face, MAX * sizeof(float)); 
    strcpy(reg->id_pessoa,id_pessoa); 
    return reg;
}

int comparador(void *a, void *b, int pos){
    float ret;
    ret = ((treg *)a)->embedding[pos] - ((treg *)b)->embedding[pos];
    if(ret < 0){
        return -1; //a < b
    } else {
        return 1; //a > b
    }
}

double distancia(void * a, void *b){
    double diferenca, distTotal = 0;
    
    for(int i = 0; i < MAX; i++){ //Pela distância euclidiana
        diferenca = ((treg *)a)->embedding[i] - ((treg *)b)->embedding[i];
        distTotal = distTotal + (diferenca * diferenca); 
    }
    return distTotal;
}


/*Definições desenvolvedor da biblioteca*/
typedef struct _node{
    void * key;
    struct _node * esq;
    struct _node * dir;
}tnode;

typedef struct _arv{
    tnode * raiz;
    int (*cmp)(void *, void *, int);
    double (*dist) (void *, void *);
    int k;
}tarv;

/*funções desenvolvedor da biblioteca*/

void kdtree_constroi(tarv * arv, int (*cmp)(void *a, void *b, int ),double (*dist) (void *, void *),int k){
    arv->raiz = NULL;
    arv->cmp = cmp;
    arv->dist = dist;
    arv->k = k;
}

/*teste*/
void test_constroi(){
    /* declaracao de variaveis */
    tarv arv;
    tnode node1;
    tnode node2;
    float v1[2] = {9.5, 7.8};
    float v2[2] = {9.4, 7.7};

    
    node1.key = aloca_reg(v1, "Pessoa A");
    node2.key = aloca_reg(v2, "Pessoa B");


    /* chamada de funções */
    kdtree_constroi(&arv,comparador,distancia,2);

    /* testes Deu certo a alocação! */
    assert(arv.raiz == NULL);
    assert(arv.k == 2);
    assert(arv.cmp(node1.key,node2.key,0) == 1);
    assert(arv.cmp(node1.key,node2.key,1) == 1);
    assert(strcpy(((treg *)node1.key)->id_pessoa,"Pessoa A"));
    assert(strcpy(((treg *)node2.key)->id_pessoa,"Pessoa B"));
    free(node1.key);
    free(node2.key);
}

void _kdtree_insere(tnode **raiz, void * key, int (*cmp)(void *a, void *b, int),int profund, int k){
    if(*raiz == NULL){
        *raiz = malloc(sizeof(tnode));
        (*raiz)->key = key;
        (*raiz)->esq = NULL;
        (*raiz)->dir = NULL;
    }else{
        int pos = profund % k;
        if (cmp( (*(*raiz)).key , key ,pos) <0){
            _kdtree_insere( &((*(*raiz)).dir),key,cmp,profund + 1,k);
        }else{
            _kdtree_insere( &((*raiz)->esq),key,cmp,profund +1,k);
        }
    }
}

void kdtree_insere(tarv *arv, void *key){
    _kdtree_insere(&(arv->raiz),key,arv->cmp,0,arv->k);
}


void _kdtree_destroi(tnode * node){
    if (node!=NULL){
        _kdtree_destroi(node->esq);
        _kdtree_destroi(node->dir);
        free(node->key);
        free(node);
    }
}

void kdtree_destroi(tarv *arv){
    _kdtree_destroi(arv->raiz);
}


void _kdtree_busca(tarv *arv, tnode ** atual, void * key, int profund, double *menor_dist, tnode **menor){
    tnode ** lado_principal; 
    tnode ** lado_oposto;    
    if (*atual != NULL){
        double dist_atual = arv->dist((*atual)->key, key);
        if (dist_atual < *menor_dist){
            *menor_dist = dist_atual;
            *menor = *atual;
        }
        int pos = profund % arv->k;
        int comp = arv->cmp(key, (*atual)->key, pos);

        printf("%s dist %4.3f menor_dist %4.3f comp %d\n", ((treg *)((tnode *)*atual)->key)->id_pessoa,dist_atual,*menor_dist,comp);

        //define lado principal para buscar
        if (comp < 0){
            lado_principal =  &((*atual)->esq);
            lado_oposto    =  &((*atual)->dir); 
        }else{
            lado_principal =  &((*atual)->dir);
            lado_oposto    =  &((*atual)->esq); 
        }

        _kdtree_busca(arv, lado_principal, key, profund + 1, menor_dist, menor);

        //Verifica se deve buscar também no outro lado

        if (comp*comp < *menor_dist) {
            //printf("tentando do outro lado %f\n",comp*comp);
            _kdtree_busca(arv, lado_oposto, key, profund + 1, menor_dist, menor);
        }
    }
}


tnode * kdtree_busca(tarv *arv, void * key){
    tnode * menor = NULL;
    double menor_dist = DBL_MAX;
    _kdtree_busca(arv,&(arv->raiz),key,0,&menor_dist,&menor);
    return menor;
}

treg buscar_mais_proximo(tarv *arv, treg query) {
    double menor_dist = 1e20;
    tnode *menor = NULL;
    _kdtree_busca(arv, &(arv->raiz), &query, 0, &menor_dist, &menor);
    return *((treg *)(menor->key));
}


tarv arvore_global;

tarv* get_tree() {
    return &arvore_global;
}

void inserir_ponto(treg p) {
    treg *novo = malloc(sizeof(treg));
    *novo = p;  // cópia de estrutura
    kdtree_insere(&arvore_global,novo);
}
void kdtree_construir() {
    arvore_global.k = 2;
    arvore_global.dist = distancia;
    arvore_global.cmp = comparador;
    arvore_global.raiz = NULL;
}

void test_busca(){
    tarv arv;
    float va[MAX] = {10,10}, vb[MAX] = {20,20}, vc[MAX] = {1,10}, vd[MAX] = {3,5}, ve[MAX] = {7,15}, vf[MAX] = {4,11};
    float vetorAtual[MAX] = {7, 14};
    
    kdtree_constroi(&arv,comparador,distancia,2);
    kdtree_insere(&arv,aloca_reg(va, "pessoa a"));
    kdtree_insere(&arv,aloca_reg(vb, "pessoa b"));
    kdtree_insere(&arv,aloca_reg(vc, "pessoa c"));
    kdtree_insere(&arv,aloca_reg(vd, "pessoa d"));
    kdtree_insere(&arv,aloca_reg(ve, "pessoa e"));
    kdtree_insere(&arv,aloca_reg(vf, "pessoa f"));
    tnode * raiz = arv.raiz;
    assert(strcmp(((treg *)raiz->dir->key)->id_pessoa, "pessoa b")==0);
    assert(strcmp(((treg *)raiz->esq->key)->id_pessoa, "pessoa c")==0);
    assert(strcmp(((treg *)raiz->esq->esq->key)->id_pessoa, "pessoa d")==0);
    assert(strcmp(((treg *)raiz->esq->dir->key)->id_pessoa, "pessoa e")==0);

    printf("\n");
    treg  * atual = aloca_reg(vetorAtual, "pessoa x");
    tnode * mais_proximo = kdtree_busca(&arv,atual);
    assert(strcmp(((treg *)mais_proximo->key)->id_pessoa,"pessoa e") == 0);

    printf("\n");
    atual->embedding[0] = 9;
    atual->embedding[1] = 9;
    mais_proximo = kdtree_busca(&arv,atual);
    assert(strcmp(((treg *)mais_proximo->key)->id_pessoa,"pessoa a") == 0);

    printf("\n");
    atual->embedding[0] = 4;
    atual->embedding[1] = 5;
    mais_proximo = kdtree_busca(&arv,atual);
    assert(strcmp(((treg *)mais_proximo->key)->id_pessoa,"pessoa d") == 0);

    printf("\n");
    atual->embedding[0] = 4;
    atual->embedding[1] = 9;
    mais_proximo = kdtree_busca(&arv,atual);
    assert(strcmp(((treg *)mais_proximo->key)->id_pessoa,"pessoa f") == 0);



    free(atual);
    kdtree_destroi(&arv);
}


int main(void){
    test_constroi(); 
    test_busca();
    printf("SUCCESS!!\n");
    return EXIT_SUCCESS;
}