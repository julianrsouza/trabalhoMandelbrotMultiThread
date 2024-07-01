#include <SDL2/SDL.h>
#include <thread>
#include <complex>
#include <iostream>


const int LARGURA = 1280;
const int ALTURA = 720;
const float POS_INICIAL = -0.5f;
const float ZOOM_INICIAL = ( LARGURA * 0.25296875f );
const float SAIR = 2.0f;
const float FATOR_ZOOM = 4.0f;

using namespace std;

const int numThreads = thread::hardware_concurrency() + 20;

int calculaMaximoIteracoes( long double zoom ) {
	return ( LARGURA / 2 ) * 0.06L * log10( zoom );
}

void renderizaFaixa( int indice, long double zoom, complex< long double > centro, SDL_Surface* superficie ) {
    int n;
    int x;
    int y;

    int maximoIteracoes = calculaMaximoIteracoes( zoom );
    complex< long double > z;
    complex< long double > c;

    long double Cor;
    int pulos = numThreads;

    for ( y = indice * ( ALTURA / pulos ); y < ( indice + 1 ) * ( ALTURA / pulos ); ++y ) {
        for ( x = 0; x < LARGURA; ++x ) {
            z = c = real( centro ) + ( ( x - ( LARGURA / 2 ) ) / zoom )
                + ( ( imag( centro ) + ( ( y - ( ALTURA / 2 ) ) / zoom ) )
                * complex<long double>( 0.0L, 1.0L ) );

            if ( ( pow( real( z ) - 0.25L, 2) + pow( imag( z ), 2 ) )
                * ( pow( real( z ), 2 ) + (x / 2L) + pow( imag( z ), 2 ) - 0.1875L )
                < pow( imag( z ), 2 ) / 4L || pow( real( z ) + 1.0L, 2 )
                + pow( imag( z ), 2 ) < 0.0625L ) {
                n = maximoIteracoes;
            }
            else {
                for (n = 0; n <= maximoIteracoes && abs(z) < SAIR; ++n) {
                    z = pow(z, 2) + c;
                }
            }

            Cor = n - log2l( logl( abs( z ) ) / 0.69314718055994530942L );
            ( ( Uint32* ) superficie->pixels ) [ ( y * superficie->w ) + x ] = ( n >= maximoIteracoes ) ?
                0 : SDL_MapRGB( superficie->format,
                    (1 + sin( Cor * 0.07L + 5 ) ) * 127.0L,
                    (1 + cos( Cor * 0.05L ) ) * 127.0L,
                    (1 + sin( Cor * 0.05L ) ) * 127.0L );
        }
    }
}

void renderizaMandelbrotMultiThread( SDL_Window* janela, SDL_Surface* superficie, complex< long double > centro, long double zoom ) {
	thread threads[ ALTURA ];
	int pulos = numThreads;
	for ( int i = 0; i < pulos; ++i ) {
		threads[ i ] = thread( renderizaFaixa, i, zoom, centro, superficie );
	}
	for ( int i = 0; i < numThreads; ++i ) {
		threads[ i ].join();
	}
	SDL_UpdateWindowSurface( janela );
}

SDL_Window* obtemJanela() {
	return SDL_CreateWindow( 
		"Mandelbrot Set", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, LARGURA, ALTURA, SDL_WINDOW_VULKAN );
}

void encerraProcesso( SDL_Window* janela ) {
	SDL_DestroyWindow( janela );
	SDL_Quit();
}


int main ( int argc, char* argv[] ) {
	SDL_Init( SDL_INIT_EVERYTHING );
	SDL_Window* janela = obtemJanela();
	if ( !janela ) {
		cerr << "Não foi possível criar a janela: " << SDL_GetError() << endl;
		SDL_Quit();
		return 1;
	}
	SDL_Surface* superficie = SDL_GetWindowSurface( janela );
	if ( !superficie ) {
		cerr << "Não foi possível criar a superficie: " << SDL_GetError() << endl;
		SDL_DestroyWindow( janela );
		SDL_Quit();
		return 1;
	}
	complex< long double > centro = POS_INICIAL;
	long double zoom = ZOOM_INICIAL;
	renderizaMandelbrotMultiThread( janela, superficie, centro, zoom );

	SDL_Event evento;

	bool manterFluxoRodando = true; 

	while ( manterFluxoRodando ) {
		SDL_PollEvent( &evento );

		switch ( evento.type ) {
		case SDL_QUIT:
			manterFluxoRodando = false;
		case SDL_KEYDOWN:
			if ( evento.key.keysym.sym == ' ' ) {
				centro = POS_INICIAL;
				zoom = ZOOM_INICIAL;
				renderizaMandelbrotMultiThread( janela, superficie, centro, zoom );
			}
			else if ( evento.key.keysym.sym == SDLK_ESCAPE ) {
				manterFluxoRodando = false;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			centro = real( centro ) + ( ( evento.button.x - ( LARGURA / 2 ) ) / zoom ) + ( ( imag( centro ) + ( ( evento.button.y - ( ALTURA / 2 ) ) / zoom ) ) * complex< long double >( 0.0L, 1.0L ) );

			if ( evento.button.button == 1 ) {
				zoom *= FATOR_ZOOM + log10( zoom );
			}
			else if ( evento.button.button == 3 ) {
				zoom /= FATOR_ZOOM;
			}
			renderizaMandelbrotMultiThread( janela, superficie, centro, zoom );
		}
	}

	encerraProcesso( janela );
	return 0;
}
