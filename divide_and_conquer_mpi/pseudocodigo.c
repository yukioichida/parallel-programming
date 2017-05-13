
MPI_Init();
my_rank = MPI_Comm_rank();  // pega pega o numero do processo atual (rank)

// recebo vetor

if ( my_rank != 0 ){
   MPI_Recv ( vetor, pai);                       // não sou a raiz, tenho pai
   MPI_Get_count(&Status, MPI_INT, &tam_vetor);  // descubro tamanho da mensagem recebida
} else {
   tam_vetor = VETOR_SIZE;               // defino tamanho inicial do vetor
   Inicializa ( vetor, tam_vetor );      // sou a raiz e portanto gero o vetor - ordem reversa
}

// dividir ou conquistar?

if ( tam_vetor <= delta )
   BubbleSort (vetor);  // conquisto
else
       {
    // dividir
    // quebrar em duas partes e mandar para os filhos

    MPI_Send ( &vetor[0], filho esquerda, tam_vetor/2 );  // mando metade inicial do vetor
    MPI_Send ( &vetor[tam_vetor/2], filho direita , tam_vetor/2 );  // mando metade final 

    // receber dos filhos

    MPI_Recv ( &vetor[0], filho esquerda);             
    MPI_Recv ( &vetor[tam_vetor/2], filho direita);    

    // intercalo vetor inteiro 
  
    Intercala ( vetor );
    }

// mando para o pai

if ( my_rank !=0 )
   MPI_Send ( vetor, pai, tam_vetor );  // tenho pai, retorno vetor ordenado pra ele
else
   Mostra ( vetor );                    // sou o raiz, mostro vetor

MPI_Finalize();

