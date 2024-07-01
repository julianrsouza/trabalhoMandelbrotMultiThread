#include <SDL2/SDL.h>
#include <complex>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;
//definição das variáveis utilizadas no código.
const unsigned int LARGURA = 1280;
const unsigned int ALTURA = 720;
const unsigned int ITERACAOMAXIMA = 1000;
const unsigned int NUMEROTHREADS = thread::hardware_concurrency();

//função que calcula o número de iterações pra um ponto no conjunto de mandelbrot.
int mandelbrot( complex<double> c ) {
	complex< double > z = 0;
	int n = 0;
	while ( abs( z ) <= 2 && n < ITERACAOMAXIMA ) {
		z = z * z + c;
		++n;
	}
	return n;
}

//função que renderiza faixa horizontal de imagem.
void renderizaFaixa( SDL_Renderer* renderizador, int comeco_y, int fim_y ) {
	for ( int y = comeco_y; y < fim_y; y++ ) {
		for ( int x = 0; x < LARGURA; x++ ) {
			double real = ( x - LARGURA / 2.0 ) * 4.0 / LARGURA;
			double imag = ( y - ALTURA / 2.0 ) * 4.0 / ALTURA;
			complex< double > c( real, imag );
			int m = mandelbrot( c );
			int cor = 255 * m / ITERACAOMAXIMA;
			SDL_SetRenderDrawColor( renderizador, cor, cor, cor, 255 );
			SDL_RenderDrawPoint( renderizador, x, y );
		}
	}
}

//função que divide o trabalho entre as threads.
void renderizaMultiThread( SDL_Renderer* renderizador ) {
	vector<std::thread> threads;
    int alturaLinha = ALTURA / NUMEROTHREADS;
    for (unsigned int i = 0; i < NUMEROTHREADS; i++) {
        int comeco_y = i * alturaLinha;
        int fim_y = (i + 1) * alturaLinha;
        threads.emplace_back(renderizaFaixa, renderizador, comeco_y, fim_y);
    }
    for (auto& t : threads) {
        t.join();
    }
	SDL_RenderPresent( renderizador );
}

//função que inicializa o SDL, cria a janela e renderizador, chama a função de renderização e mantém a janela aberta.
int run() {
	if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		cerr << "Não foi possível inicializar o SDL: " << SDL_GetError() << endl;
		return 1;
	}
	cout << "SDL inicializado com sucesso!" << endl;

	//criação da janela.
	SDL_Window* janela = SDL_CreateWindow( 
		"Mandelbrot Set", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, LARGURA, ALTURA, SDL_WINDOW_SHOWN );

	if ( !janela ) {
		cerr << "Não foi possível criar a janela: " << SDL_GetError() << endl;
		SDL_Quit();
		return 1;
	}
	cout << "Janela criada com sucesso!" << endl;

	//criação do renderizador.
	SDL_Renderer* renderizador = SDL_CreateRenderer( janela, -1, SDL_RENDERER_ACCELERATED );
	if ( !renderizador ) {
		cerr<< "Não foi possível criar renderizador: " << SDL_GetError() << endl;
		SDL_DestroyWindow( janela );
		SDL_Quit();
		return 1;
	}
	cout << "Renderizador criado com sucesso!" << endl;

	//chamada da função/método de renderização multithread.
	renderizaMultiThread( renderizador );
	//criação da variável que mantém o loop de execução.
	bool sair = false;
	//instanciação do evento SDL utilizado no loop.
	SDL_Event evento;

	//loop de execução que mantém a janela aberta.
	while( !sair ) {
		while( SDL_PollEvent( &evento )) {
			//validacao do tipo do evento SDL, se for igual a SLD_QUIT, ele trocará a variável de controle do loop pra true,
			//que fará o while se encerrar.
			if ( evento.type == SDL_QUIT ) {
				sair = true;
			}
		}
	}

	//encerramento do processo, destruindo o renderizador, a janela e encerrando o SDL. 
	SDL_DestroyRenderer( renderizador );
	SDL_DestroyWindow( janela );
	SDL_Quit();
	cout << "Recursos da SDL destruídos e SDL encerrado." << endl;

	return 0;
}

//função main padrão.
int main( int argc, char* argv[] ) {
	return run();
}