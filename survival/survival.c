#include <SDL.h>
#include <stdio.h>
#include <SDL_image.h>
// la mayoria de datos SDL, se reprecentan de la siguiente manera:
// SDL_tipoDeDatoVariable *nombreReprecentativo = FuncionReservada (parametros) 
// algunas veces el igual no hace falta, por ejemplo los casos de eventos, ya que solo necesitamos tener/nombrar una variable para acceder a ellos ya que
// son recolectados de manera transparente al programador.
// * es para declarar punteros o acceder a ellos, mientras que & es para apuntar a la direccion de memoria, en si por ejemplo *p y &p son distintos, 
// *p sera el contenido o la definicion del puntero, y &p sera la direccion de memoria del puntero, usualmente en hexadesimal
// si realizas *p = 10, el 10 sera la direccion de memoria en hex, no un valor numerico normalmente tratable, si realizas x = *p, x tendra el contenido de la dir 10
typedef struct {
    int x, y;               // Posición del disparo
    int ancho, alto;        // Tamaño del disparo
    int activo;             // Si el disparo está en pantalla
    int direccion_x, direccion_y; // Dirección de movimiento
} Disparo;

// Definición de estructuras para representar enemigos y disparos en el juego
typedef struct {
    int x, y;                    // Posición
    int ancho, alto;             // Tamaño
    int activo;                  // Estado activo/inactivo
    int velocidad_x;             // Velocidad horizontal
    int frameActual;             // Frame actual de la animación
    int tiempoFrame;             // Duración de cada frame en ms
    Uint32 tiempoAnterior;       // Marca de tiempo del último frame
    Uint32 tiempoReaparicion;    // Tiempo de reaparición después de ser desactivado
} Enemigo1;

typedef struct {
    int x, y;                    // Posición
    int ancho, alto;             // Tamaño
    int activo;                  // Estado activo/inactivo
    int velocidad_x;             // Velocidad horizontal
    int frameActual;             // Frame actual de la animación
    int tiempoFrame;             // Duración de cada frame en ms
    Uint32 tiempoAnterior;       // Marca de tiempo del último frame
    Uint32 tiempoReaparicion;    // Tiempo de reaparición después de ser desactivado
} Enemigo2;

typedef struct {
	int x, y;            // Posición en la pantalla
    int ancho, alto;   // Dimensiones del jugador
    int salud;           // Salud del jugador
    int dinero;          //Dinero del jugador
    int velocidad;       // Velocidad de movimiento
    int ultima_direccion_x;  //valores de resguardo para direccionar las balas en un sentido u otro
    int ultima_direccion_y;
} Jugador;


Uint32 tiempoAtaqueEnemigo2 = 0;     // Tiempo del último ataque del enemigo 2
Uint32 cooldownAtaqueEnemigo2 = 750; // Tiempo entre ataques del enemigo 2
int atacandoEnemigo2 = 0;            // Estado de ataque del enemigo 2

// Constantes para controlar las animaciones, velocidad y dimensiones de los sprites
#define FRAMES_PROYECTIL_ENEMIGO2 6
#define FRAMES_ATAQUE_ENEMIGO2 4
#define VELOCIDAD_PROYECTIL_ENEMIGO2 8
#define LIMITE_ATAQUE_X 0.4 // Límite de ataque como porcentaje de ancho de la pantalla
#define ANCHO_FRAME 32
#define ALTO_FRAME 32
#define FRAMES_REPOSO 4
#define FRAMES_CAMINAR 4
#define FRAMES_DISPARO 4
#define FRAMES_ENEMIGO1 4
#define FRAMES_ENEMIGO2 3
#define VELOCIDAD_DISPARO 10
#define TIEMPO_REAPARICION_ENEMIGO 2000 // Tiempo de reaparición del enemigo en ms
#define MAX_DISPAROS 10

Disparo disparosEnemigo2[MAX_DISPAROS];
// Enumeración para controlar el tipo de animación
typedef enum {
    REPOSO,
    CAMINAR,
    DISPARO,
    ENEMIGO1_ANIMACION,
    ENEMIGO2_ANIMACION
} Animacion;

// Tiempos para la duración de cada tipo de animación
int tiempoFrameReposo = 150;
int tiempoFrameCaminar = 400;

// Función para detectar colisiones entre dos rectángulos
int detectarColision(SDL_Rect a, SDL_Rect b) {
    return (a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y);
}


int main(int argc, char *argv[]) {
	
    // Inicializar SDL y SDL_image
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("No se pudo inicializar SDL: %s\n", SDL_GetError());
        return 1;
    }
    
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("No se pudo inicializar SDL_image: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    // Crear una ventana, 1280 para notebook, 1024 normal, 800 peque, el alto 600 esta al limite mas o menos
    //los parametros de createwindow son (nombre de ventana, posicion x, posicion y, ancho de ventana, alto de ventana, mostrar al crear)
    // asigno al tipo de variable SDL visual que reprecenta una ventana, bajo el nombre ventana
    
    SDL_Window *ventana = SDL_CreateWindow("Survival",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,1280, 600,SDL_WINDOW_SHOWN);
    //tratamiento de exepcion
	if (!ventana){
        printf("No se pudo crear la ventana: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Crear un renderer para dibujar
    //el renderer nos asegura de poder visualizar cualquier cosa posible a dibujar, la "ventana" por si sola no puede visualizarse solo la creamos de manera
    // "especial" con SDL_window para darle un "privilegio" o "jerarquia" a todas las demas figuras a futuro que se realicen, de esa manera es mas sencilla
    // desarrollar logica entro "entidades" o "dibujos" para que no se "pasen" del borde de la ventana definida, limitando los movimientos a producirce solo
    // dentro de la ventana creada y propuesta.
    // los parametros significan (a quien quiero renderizar, utilizacion de gpu u otro motor grafico hardware, opciones de aceleracion grafica)
    SDL_Renderer *renderer = SDL_CreateRenderer(ventana, -1, SDL_RENDERER_ACCELERATED);
    //tratamiento de exepcion
    if (!renderer) {
        printf("No se pudo crear el renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(ventana);
        SDL_Quit();
        return 1;
    }

	// Cargar texturas de los personajes, enemigos y proyectiles
    SDL_Texture *jugadorTextura = IMG_LoadTexture(renderer, "sprites/personaje2.png");
    SDL_Texture *enemigo1Textura = IMG_LoadTexture(renderer, "sprites/enemigo1.png");
    SDL_Texture *enemigo2Textura = IMG_LoadTexture(renderer, "sprites/enemigo2.png");
    SDL_Texture *proyectilEnemigo2Textura = IMG_LoadTexture(renderer, "imagenes/proyectilEnemigo2.png");
    SDL_Texture *balaTextura = IMG_LoadTexture(renderer, "sprites/bala.png");
    //img de fondo
    SDL_Texture *fondoTextura = IMG_LoadTexture(renderer, "sprites/fondo.png");
    //exepcion para el fondo
    if (!fondoTextura) {
        printf("No se pudo cargar la textura de fondo: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(ventana);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    //inicializacion de registro o entidad jugador
	Jugador player = {640, 300, 80, 80, 100, 0, 5, 1, 0}; //pos x, pos y, ancho, alto, salud, dinero, velocidad, x ant, y ant
  	SDL_Rect jugador = {player.x, player.y, player.ancho, player.alto};
  	
	 // Inicializar los disparos
    Disparo disparos[MAX_DISPAROS];
    int i= 0;
    for ( i = 0; i < MAX_DISPAROS; i++) {
        disparos[i].activo = 0;
    }
    
    for (i = 0; i < MAX_DISPAROS; i++) {
    disparosEnemigo2[i].activo = 0;
	}
    
     // Variables para el cooldown de disparo
    Uint32 ultimo_disparo = 0;
    Uint32 cooldown = 500; // 500 milisegundos
    Enemigo1 enemigo1 = {1000, 500, 80, 80, 1, 4, 0, 200, SDL_GetTicks(), 0};
    Enemigo2 enemigo2 = {1000, 300, 80, 80, 1, 3, 0, 200, SDL_GetTicks(), 0};
    
    int frameActual = 0;
    int tiempoFrame = 100;
    Uint32 tiempoAnterior = SDL_GetTicks();
    int enMovimiento = 0;
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    Animacion animacionActual = REPOSO;
    Uint32 tiempoInicioDisparo = 0;
    int duracionAnimacionDisparo = 400;

   // Bucle de eventos para mantener la ventana abierta
   
    int running = 1; //variable identificatoria para realizar bucle de muestreo/actualizacion hasta que el usuario intervenga
    SDL_Event evento;//asigno un nombre a los tipos de eventos que reacciona SDL_event, alli pararían todos los eventos por raton o teclado recolectados
    // los eventos pueden ser de raton, teclado o joystick, todos tienen palabras reservadas down y up, referenciando la accion de apretar y soltar respectivamente
    // se debe configurar ambos de lo contrario la accion se volvera perpetua
   int velocidad = 5; //velocidad traducida en 5 pixeles por bucle segun x accion de desplazamiento en cada eje
   //int ultima_direccion_x = 1;
   //int ultima_direccion_y = 0;
   Disparo disparo = {0, 0, 6, 2, 0, 0, 0}; // inicializamos la entidad disparo desactivada, los parametros son correspondientes y posicionales al struct
   
    while (running) {
        // Procesar eventos
        // SDL_PollEvent es algo asi como un CACHE de eventos captados por la "ventana" renderizada por SDL
        enMovimiento = 0;
        //bucle administrador de eventos
        while (SDL_PollEvent(&evento)) {
        	//recorre la cola de eventos y realiza opciones segun corresponda
        	if (evento.type == SDL_QUIT) {
                running = 0;
            } else if (evento.type == SDL_KEYDOWN) {
                switch (evento.key.keysym.sym) {
                    case SDLK_w: 
                    
                         	player.y -= player.velocidad;
                         	enMovimiento = 1;
                        	animacionActual = CAMINAR;
						 	player.ultima_direccion_x = 0; 
                        	player.ultima_direccion_y = -1;
                        	
						break;
						
                    case SDLK_s:
                    	
                       	player.y += player.velocidad;
                       	enMovimiento = 1;
                        animacionActual = CAMINAR;
                        player.ultima_direccion_x = 0; 
                        player.ultima_direccion_y = 1;
                        break;
                        
                    case SDLK_a: 
                    
                        player.x -= player.velocidad;
                        enMovimiento = 1;
                        flip = SDL_FLIP_HORIZONTAL;
                        animacionActual = CAMINAR;
                    	player.ultima_direccion_x = -1; 
                        player.ultima_direccion_y = 0;
						break;
						
                    case SDLK_d: 
                    
                       	player.x += player.velocidad;
                       	enMovimiento = 1;
                        flip = SDL_FLIP_NONE;
                        animacionActual = CAMINAR;
						player.ultima_direccion_x = 1; 
                        player.ultima_direccion_y = 0;
                        break;
                        
                    case SDLK_j:// disparo con la letra "j"
                            if (SDL_GetTicks() > ultimo_disparo + cooldown) {
                                for (i = 0; i < MAX_DISPAROS; i++) {
                                    if (!disparos[i].activo) { // Encontrar un disparo inactivo
                                        disparos[i].activo = 1;
                                        disparos[i].x = player.x + player.ancho / 2 - 3; // Centrar el disparo
                                        disparos[i].y = player.y + player.alto / 2 - 1;
                                        disparos[i].ancho = 20;
                                        disparos[i].alto = 20;
                                        disparos[i].direccion_x = player.ultima_direccion_x;
                                        disparos[i].direccion_y = player.ultima_direccion_y;
                                        ultimo_disparo = SDL_GetTicks();
                                    	animacionActual = DISPARO;
                                    	tiempoInicioDisparo = SDL_GetTicks();
                                        break; // Solo un disparo por pulsación
                                    }
                                }
                            }
                		break;
                }
            }
        }
        
        // Control de animaciones del jugador
        if (animacionActual == DISPARO && SDL_GetTicks() > tiempoInicioDisparo + duracionAnimacionDisparo) {
            animacionActual = REPOSO;
        }

        // Actualizar frame de animación
        Uint32 tiempoActual = SDL_GetTicks();

        if (animacionActual == CAMINAR) {
		    // Ralentizamos la animación al caminar
		    if (tiempoActual > tiempoAnterior + tiempoFrameCaminar) {
		        frameActual = (frameActual + 1) % FRAMES_CAMINAR;
		        tiempoAnterior = tiempoActual;
		    }
		} else if (animacionActual == REPOSO) {
		    // Ralentizamos la animación de reposo
		    if (tiempoActual > tiempoAnterior + tiempoFrameReposo) {
		        frameActual = (frameActual + 1) % FRAMES_REPOSO;
		        tiempoAnterior = tiempoActual;
		    }
		} else {
		    // Para otras animaciones, puedes dejar el tiempo por defecto
		    if (tiempoActual > tiempoAnterior + tiempoFrame) {
		        frameActual = (frameActual + 1) % FRAMES_REPOSO;
		        tiempoAnterior = tiempoActual;
		    }
		}



        // Movimiento de disparos
        for (i = 0; i < MAX_DISPAROS; i++) {
            if (disparos[i].activo) {
                disparos[i].x += disparos[i].direccion_x * VELOCIDAD_DISPARO;
                if (disparos[i].x < 0 || disparos[i].x > 1280) {
                    disparos[i].activo = 0;
                }
            }
        }
        
        // actualizacion de enemigo 1
        if (enemigo1.activo) {
            enemigo1.x -= enemigo1.velocidad_x;
            if (enemigo1.x < 0) enemigo1.x = 1024 - enemigo1.ancho;
            if (tiempoActual > enemigo1.tiempoAnterior + enemigo1.tiempoFrame) {
                enemigo1.frameActual = (enemigo1.frameActual + 1) % FRAMES_ENEMIGO1;
                enemigo1.tiempoAnterior = tiempoActual;
            }
        } else if (SDL_GetTicks() > enemigo1.tiempoReaparicion) {
            enemigo1.activo = 1;
            enemigo1.x = 1024;
            enemigo1.y = rand() % 600;
        }
        
        // actualizacion de reaparicion de enemigo 2
        if (!enemigo2.activo && SDL_GetTicks() > enemigo2.tiempoReaparicion) {
    		printf("Reapareciendo enemigo 2\n"); // Mensaje de depuración
    		enemigo2.activo = 1;
    		enemigo2.x = 1024;
    		enemigo2.y = rand() % 600;
    		atacandoEnemigo2 = 0;
    		enemigo2.frameActual = 0;
		}
		
		//logica de animacion y ataque automatico de enemigo 2, el cual ataca a distancia
		
		if (enemigo2.activo) {
    		// Movimiento del enemigo
    		if (!atacandoEnemigo2) {
        		enemigo2.x -= enemigo2.velocidad_x;
    		}

    		if (!atacandoEnemigo2 && enemigo2.x < 1024 * (1 - LIMITE_ATAQUE_X)) {
        		atacandoEnemigo2 = 1;
        		tiempoAtaqueEnemigo2 = SDL_GetTicks();
        		enemigo2.frameActual = 0;
    		}
			//logica de disparos de enemigo 2
    		if (atacandoEnemigo2) {
		        if (SDL_GetTicks() > enemigo2.tiempoAnterior + enemigo2.tiempoFrame) {
    		        enemigo2.frameActual = (enemigo2.frameActual + 1) % FRAMES_ATAQUE_ENEMIGO2;
        		    enemigo2.tiempoAnterior = SDL_GetTicks();
        		}	
		
	        	if (SDL_GetTicks() > tiempoAtaqueEnemigo2 + cooldownAtaqueEnemigo2) {
    	        	for (i = 0; i < MAX_DISPAROS; i++) {
        	        	if (!disparosEnemigo2[i].activo) {
            	        	disparosEnemigo2[i].activo = 1;
                	    	disparosEnemigo2[i].x = enemigo2.x;
                    		disparosEnemigo2[i].y = enemigo2.y + enemigo2.alto / 2;
	                    	disparosEnemigo2[i].ancho = 32;
    	                	disparosEnemigo2[i].alto = 32;
        	            	disparosEnemigo2[i].direccion_x = -1;
            	        	tiempoAtaqueEnemigo2 = SDL_GetTicks();
                	    	break;
                		}
	            	}
    	    	}
    		} else {//actualizacion de contador para realizar frames de animaciones
    			if (SDL_GetTicks() > enemigo2.tiempoAnterior + enemigo2.tiempoFrame) {
	            enemigo2.frameActual = (enemigo2.frameActual + 1) % FRAMES_ENEMIGO2;
    	        enemigo2.tiempoAnterior = SDL_GetTicks();
        		}
    		}
    	
	    	//si la figura del enemigo exede cierto valor de pantalla, se desactiva
    		if (enemigo2.x < -enemigo2.ancho) {
		        printf("Desactivando enemigo 2\n"); // Mensaje de depuración
	        	enemigo2.activo = 0;
	        	atacandoEnemigo2 = 0;
	        	enemigo2.tiempoReaparicion = SDL_GetTicks() + TIEMPO_REAPARICION_ENEMIGO;
	    	}
    	}
		
		SDL_RenderCopy(renderer, fondoTextura, NULL, NULL);
		
		// Punto de referencia
		// Aquí es donde debes agregar el bloque del punto 4, antes de renderizar al jugador
		
		for (i = 0; i < MAX_DISPAROS; i++) {
    		if (disparosEnemigo2[i].activo) {
        		disparosEnemigo2[i].x += disparosEnemigo2[i].direccion_x * VELOCIDAD_PROYECTIL_ENEMIGO2;
	        	
				// Desactivar el proyectil si sale de la pantalla
    	   		
				if (disparosEnemigo2[i].x < 0 || disparosEnemigo2[i].x > 1024) {
        	    	disparosEnemigo2[i].activo = 0;
	        	}
	        
	        	// Renderizar el proyectil
		        SDL_Rect proyectilRect = {disparosEnemigo2[i].x, disparosEnemigo2[i].y, disparosEnemigo2[i].ancho, disparosEnemigo2[i].alto};
				proyectilRect.w = 64; // Ancho deseado
        		proyectilRect.h = 64;
				// Asume que el proyectil tiene varias frames horizontales en el spritesheet
				int frameProyectil = (SDL_GetTicks() / 100) % FRAMES_PROYECTIL_ENEMIGO2;  // Ajusta 100 para cambiar la velocidad de animación
				SDL_Rect srcProyectilEnemigo2 = {frameProyectil * ANCHO_FRAME, 0, ANCHO_FRAME, ALTO_FRAME};
				SDL_RenderCopy(renderer, proyectilEnemigo2Textura, &srcProyectilEnemigo2, &proyectilRect);
        	}
		}
        
        // Actualización de posición del rectángulo de representación
		jugador.x = player.x;
		jugador.y = player.y;
		jugador.w = player.ancho;
		jugador.h = player.alto;
        
		// Renderizado del jugador
		
		SDL_Rect srcRect = {frameActual * ANCHO_FRAME, animacionActual * ALTO_FRAME, ANCHO_FRAME, ALTO_FRAME};
		SDL_Rect dstRect = {jugador.x, jugador.y, jugador.w, jugador.h};
		SDL_RenderCopyEx(renderer, jugadorTextura, &srcRect, &dstRect, 0, NULL, flip);
		
		for (i = 0; i < MAX_DISPAROS; i++) {
            if (disparos[i].activo) {
                SDL_Rect rect_disparo = {disparos[i].x, disparos[i].y, disparos[i].ancho, disparos[i].alto};
                SDL_RenderCopy(renderer, balaTextura, NULL, &rect_disparo);

                SDL_Rect enemigo1Rect = {enemigo1.x, enemigo1.y, enemigo1.ancho, enemigo1.alto};
                if (enemigo1.activo && detectarColision(rect_disparo, enemigo1Rect)) {
                    disparos[i].activo = 0;
                    enemigo1.activo = 0;
                    enemigo1.tiempoReaparicion = SDL_GetTicks() + TIEMPO_REAPARICION_ENEMIGO;
                }

                SDL_Rect enemigo2Rect = {enemigo2.x, enemigo2.y, enemigo2.ancho, enemigo2.alto};
                if (enemigo2.activo && detectarColision(rect_disparo, enemigo2Rect)) {
                    disparos[i].activo = 0;
                    enemigo2.activo = 0;
                    enemigo2.tiempoReaparicion = SDL_GetTicks() + TIEMPO_REAPARICION_ENEMIGO;
                }
            }
        }
        
        if (enemigo1.activo) {
            SDL_Rect enemigo1SrcRect = {enemigo1.frameActual * ANCHO_FRAME, 0, ANCHO_FRAME, ALTO_FRAME};
            SDL_Rect enemigo1DstRect = {enemigo1.x, enemigo1.y, enemigo1.ancho, enemigo1.alto};
            SDL_RenderCopy(renderer, enemigo1Textura, &enemigo1SrcRect, &enemigo1DstRect);
        }

        if (enemigo2.activo) {
    		SDL_Rect enemigo2SrcRect;
    
    		if (atacandoEnemigo2) {
        		enemigo2SrcRect = (SDL_Rect){enemigo2.frameActual * ANCHO_FRAME, ALTO_FRAME, ANCHO_FRAME, ALTO_FRAME};
    		} else {
        		enemigo2SrcRect = (SDL_Rect){enemigo2.frameActual * ANCHO_FRAME, 0, ANCHO_FRAME, ALTO_FRAME};
    		}

    		SDL_Rect enemigo2DstRect = {enemigo2.x, enemigo2.y, enemigo2.ancho, enemigo2.alto};
    		SDL_RenderCopy(renderer, enemigo2Textura, &enemigo2SrcRect, &enemigo2DstRect);
		}
		 SDL_RenderPresent(renderer);
        SDL_Delay(16);
	}
		
    // Liberar recursos y cerrar SDL
    SDL_DestroyTexture(proyectilEnemigo2Textura);
	SDL_DestroyTexture(fondoTextura);
    SDL_DestroyTexture(balaTextura);
    SDL_DestroyTexture(enemigo1Textura);
    SDL_DestroyTexture(enemigo2Textura);
    SDL_DestroyTexture(jugadorTextura);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(ventana);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
