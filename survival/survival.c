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
    SDL_Texture *fondoTextura = IMG_LoadTexture(renderer, "sprites/fondo.png");
    if (!fondoTextura) {
        printf("No se pudo cargar la textura de fondo: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(ventana);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

/*
	  // Establecer el color de fondo (azul)
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // RGB: azul, aplicamos azul a la entidad
    SDL_RenderClear(renderer); // Llenar con el color establecido, borramos todo lo que tenia la entidad y aplica cambios AUN NO VISIBLES
    SDL_RenderPresent(renderer); // Mostrar en pantalla LOS CAMBIOS NO VISIBLES
    
    SDL_Rect rectangulo = {0, 100, 1280, 400}; // realiza un rectangulo relleno, paramentros = (coordenadas x, coordenads y, largo, alto)
    //SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);//este color se APLICARA a EL SIGUIENTE DIBUJO, si no aparece nada se aplica al FONDO o VENTANA
    //SDL_RenderFillRect(renderer, &rectangulo);// crea el dibujo RECTANGULO RELLENO, dentro de renderer con la direccion de rectangulo PERO AUN NO ES VISIBLE
    //SDL_RenderPresent(renderer);//HACE VISIBLE LOS CAMBIOS ANTERIORES
  	*/
  	SDL_Rect jugador = {640, 300, 80, 80};
  	
	 // Inicializar los disparos
    Disparo disparos[MAX_DISPAROS];
    int i= 0;
    for ( i = 0; i < MAX_DISPAROS; i++) {
        disparos[i].activo = 0;
    }
    
     // Variables para el cooldown de disparo
    Uint32 ultimo_disparo = 0;
    Uint32 cooldown = 500; // 500 milisegundos
    Enemigo1 enemigo1 = {1820, 540, 96, 96, 1, 4, 0, 200, SDL_GetTicks(), 0};
    Enemigo2 enemigo2 = {1820, 300, 96, 96, 1, 3, 0, 200, SDL_GetTicks(), 0};
    
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
   int ultima_direccion_x = 1;
   int ultima_direccion_y = 0;
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
                    case SDLK_w: // Movimiento hacia arriba
                    	/* if (jugador.y - velocidad >= rectangulo.y){
						 	jugador.y -= velocidad;
						 	ultima_direccion_x = 0; 
                        	ultima_direccion_y = -1;
                        }*/
                         	jugador.y -= velocidad;
                         	enMovimiento = 1;
                        	animacionActual = CAMINAR;
						 	ultima_direccion_x = 0; 
                        	ultima_direccion_y = -1;
                        	
						break;
						
                    case SDLK_s: // Movimiento hacia abajo
                    	/* if (jugador.y + jugador.h + velocidad <= rectangulo.y + rectangulo.h){
                        	jugador.y += velocidad;
                        	ultima_direccion_x = 0; 
                            ultima_direccion_y = 1;
						} */
                       	jugador.y += velocidad;
                       	enMovimiento = 1;
                        animacionActual = CAMINAR;
                        ultima_direccion_x = 0; 
                        ultima_direccion_y = 1;
                        break;
                        
                    case SDLK_a: // Movimiento hacia la izquierda
                    	/* if (jugador.x - velocidad >= rectangulo.x){
                        	 jugador.x -= velocidad;
                        	 ultima_direccion_x = -1; 
                             ultima_direccion_y = 0;
                    } */
                        jugador.x -= velocidad;
                        enMovimiento = 1;
                        flip = SDL_FLIP_HORIZONTAL;
                        animacionActual = CAMINAR;
                    	ultima_direccion_x = -1; 
                        ultima_direccion_y = 0;
						break;
						
                    case SDLK_d: // Movimiento hacia la derecha
                    	/*  if (jugador.x + jugador.w + velocidad <= rectangulo.x + rectangulo.w){
							jugador.x += velocidad;
							ultima_direccion_x = 1; 
                            ultima_direccion_y = 0;
                        } */
                       	jugador.x += velocidad;
                       	enMovimiento = 1;
                        flip = SDL_FLIP_NONE;
                        animacionActual = CAMINAR;
						ultima_direccion_x = 1; 
                        ultima_direccion_y = 0;
                        break;
                        
                    case SDLK_j:// disparo con la letra "j"
                            if (SDL_GetTicks() > ultimo_disparo + cooldown) {
                                for (i = 0; i < MAX_DISPAROS; i++) {
                                    if (!disparos[i].activo) { // Encontrar un disparo inactivo
                                        disparos[i].activo = 1;
                                        disparos[i].x = jugador.x + jugador.w / 2 - 3; // Centrar el disparo
                                        disparos[i].y = jugador.y + jugador.h / 2 - 1;
                                        disparos[i].ancho = 20;
                                        disparos[i].alto = 20;
                                        disparos[i].direccion_x = ultima_direccion_x;
                                        disparos[i].direccion_y = ultima_direccion_y;
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
        /*
		// Mover/actualizar el disparo si está activo
        for (i = 0; i < MAX_DISPAROS; i++) {
            if (disparos[i].activo) {
                int velocidad_disparo = 10;
                disparos[i].x += disparos[i].direccion_x * velocidad_disparo;
                disparos[i].y += disparos[i].direccion_y * velocidad_disparo;

                // Si el disparo sale de los límites del rectángulo, desactivarlo
                if (disparos[i].x < rectangulo.x || disparos[i].x > rectangulo.x + rectangulo.w ||
                    disparos[i].y < rectangulo.y || disparos[i].y > rectangulo.y + rectangulo.h) {
                    disparos[i].activo = 0;
                }
            }
            
        }*/
        
        // Control de animaciones del jugador
        if (animacionActual == DISPARO && SDL_GetTicks() > tiempoInicioDisparo + duracionAnimacionDisparo) {
            animacionActual = REPOSO;
        }

        // Actualizar frame de animación
        Uint32 tiempoActual = SDL_GetTicks();
        if (tiempoActual > tiempoAnterior + tiempoFrame) {
            frameActual++;
            tiempoAnterior = tiempoActual;
            switch (animacionActual) {
                case REPOSO:
                    frameActual %= FRAMES_REPOSO;
                    tiempoFrame = tiempoFrameReposo;
                    break;
                case CAMINAR:
                    frameActual %= FRAMES_CAMINAR;
                    tiempoFrame = tiempoFrameCaminar;
                    break;
                case DISPARO:
                    frameActual %= FRAMES_DISPARO;
                    break;
                case ENEMIGO1_ANIMACION:
                    enemigo1.frameActual = (enemigo1.frameActual + 1) % FRAMES_ENEMIGO1;
                    break;
                case ENEMIGO2_ANIMACION:
                    enemigo2.frameActual = (enemigo2.frameActual + 1) % FRAMES_ENEMIGO2;
                    break;
            }
        }

        // Control de ataque del enemigo 2
        if (enemigo2.activo && SDL_GetTicks() > tiempoAtaqueEnemigo2 + cooldownAtaqueEnemigo2 && !atacandoEnemigo2) {
            if ((float)enemigo2.x / 1920.0 <= LIMITE_ATAQUE_X) {
                atacandoEnemigo2 = 1;
                tiempoAtaqueEnemigo2 = SDL_GetTicks();
            }
        }

        // Movimiento de disparos
        for (i = 0; i < MAX_DISPAROS; i++) {
            if (disparos[i].activo) {
                disparos[i].x += disparos[i].direccion_x * VELOCIDAD_DISPARO;
                if (disparos[i].x < 0 || disparos[i].x > 1920) {
                    disparos[i].activo = 0;
                }
            }
        }
        
         // Renderizar el fondo y objetos del juego
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, fondoTextura, NULL, NULL);
        SDL_Rect srcRect = {frameActual * ANCHO_FRAME, animacionActual * ALTO_FRAME, ANCHO_FRAME, ALTO_FRAME};
        SDL_RenderCopyEx(renderer, jugadorTextura, &srcRect, &jugador, 0, NULL, flip);

        // Renderizar disparos activos
        SDL_Rect disparoRect;
        for (i = 0; i < MAX_DISPAROS; i++) {
            if (disparos[i].activo) {
                disparoRect = (SDL_Rect){disparos[i].x, disparos[i].y, disparos[i].ancho, disparos[i].alto};
                SDL_RenderCopy(renderer, balaTextura, NULL, &disparoRect);
            }
        }

        SDL_RenderPresent(renderer);
    }

        
		/*   
           // Limpiar la pantalla
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Fondo azul de la ventana principal
        SDL_RenderClear(renderer);

        // Dibujar el rectángulo límite
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // fondo negro del "puente"
        SDL_RenderFillRect(renderer, &rectangulo);

        // Dibujar el cuadrado en su posición actual
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // verde para el jugador
        SDL_RenderFillRect(renderer, &jugador);
        
        // Dibujar los disparos
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Rojo para los disparos
        for ( i = 0; i < MAX_DISPAROS; i++) {
            if (disparos[i].activo) {
                SDL_Rect rect_disparo = {disparos[i].x, disparos[i].y, disparos[i].ancho, disparos[i].alto};
                SDL_RenderFillRect(renderer, &rect_disparo);
            }
        }

        // Actualizar la pantalla
        SDL_RenderPresent(renderer);
		
        SDL_Delay(16); // Para limitar el uso de CPU (aprox. 60 FPS)
        
    }
        
        */

    // Liberar recursos y cerrar SDL
    SDL_DestroyTexture(fondoTextura);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(ventana);
    IMG_Quit();
    SDL_Quit();
    return 0;
    return 0;
}
