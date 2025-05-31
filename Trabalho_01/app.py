# app.py
from fastapi import FastAPI, Query
from kdtree_wrapper import lib, Tarv, TReg
from ctypes import POINTER,c_char, c_float
from pydantic import BaseModel

app = FastAPI()


class PontoEntrada(BaseModel):
    embedding: list[float]
    id_pessoa: str

@app.post("/construir-arvore")
def constroi_arvore():
    lib.kdtree_construir()
    return {"mensagem": "Árvore KD inicializada com sucesso."}

@app.post("/inserir")
def inserir(ponto: PontoEntrada):
    emb_vetor = (c_float * 128)(*ponto.embedding)
    id_bytes = ponto.id_pessoa.encode('utf-8')[:99]  # Trunca se necessário
    novo_ponto = TReg(embedding=emb_vetor, id_pessoa=id_bytes)
    lib.inserir_ponto(novo_ponto)
    return {"mensagem": f"Dados da pessoa '{ponto.id_pessoa}' inserido com sucesso."}

@app.get("/buscar")
def buscar(embedding: list[float] = Query(...), id_pessoa: str =  Query(...)):
    emb_vetor = (c_float * 128)(*embedding)  
    id_bytes = id_pessoa.encode("utf-8")[:99]  

    query = TReg(embedding=emb_vetor, id_pessoa=id_bytes)

    arv = lib.get_tree()  # Suponha que esta função retorne ponteiro para árvore já construída
    resultado = lib.buscar_mais_proximo(arv, query)

    return {
        "embedding": resultado.embedding,
        "id_pessoa": resultado.id_pessoa
    }
