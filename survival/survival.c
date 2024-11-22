#include <SDL.h>
#include <stdio.h>
#include <SDL_image.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <SDL_ttf.h>

void mostrarVentanaEmergente() {
    printf("");
}

// la mayoria de datos SDL, se reprecentan de la siguiente manera:
// SDL_tipoDeDatoVariable *nombreReprecentativo = FuncionReservada (parametros) 
// algunas veces el igual no hace falta, por ejemplo los casos de eventos, ya que solo necesitamos tener/nombrar una variable para acceder a ellos ya que
// son recolectados de manera transparente al programador.
// * es para declarar punteros o acceder a ellos, mientras que & es para apuntar a la direccion de memoria
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
    int vida;
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

typedef struct {
    int x, y;               // Posición de la moneda
    int ancho, alto;        // Tamaño de la moneda
    int activo;             // Si la moneda está en pantalla
} Moneda;

typedef struct {
    int numero_oleada;        // oleadas faltantes
    int oleada_maxima;         // maximo de oleada
    int enemigos_activos;     // enemigos vivos en pantalla
    Uint32 tiempo_restante;   // Tiempo restante para la oleada (en ms)
    Uint32 tiempo_transicion; // Tiempo entre oleadas (en ms)
    int en_transicion;        // 1 si está en transición, 0 si está en oleada activa
    int momento_dia; 		  // 1 dia, 2 atardecer, 3 noche, 4 amanecer
} Oleada;

typedef struct {
	int x, y;             // Posición
    int ancho, alto;      // Tamaño
    int construida;       // 0 = destruida, 1 = construida
    int vida;             // cantidad de hits soportada de la barrera
}Defensa;

Uint32 tiempoAtaqueEnemigo2 = 0;     // Tiempo del último ataque del enemigo 2
Uint32 cooldownAtaqueEnemigo2 = 750; // Tiempo entre ataques del enemigo 2
int atacandoEnemigo2 = 0;            // Estado de ataque del enemigo 2
Uint32 tiempo_actual;
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
#define TIEMPO_REAPARICION_ENEMIGO 3000 // Tiempo de reaparición del enemigo en ms
#define MAX_DISPAROS 10
#define MAX_MONEDAS 5
#define MAX_ENEMIGOS 10
#define VIDA_DEFAULT 6

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

int i;
int j;

void iniciar_oleada(Oleada *oleada_actual, Enemigo1 enemigo1[]) {
	
	oleada_actual->numero_oleada++;
    oleada_actual->enemigos_activos = 0;
    oleada_actual->tiempo_restante = 30000; // 30 segundos en milisegundos
    oleada_actual->tiempo_transicion = 15000; // 5 segundos de transición/descanso entre oleadas
    oleada_actual->en_transicion = 0;
    
    for (i = 0; i < MAX_ENEMIGOS; i++) {
        enemigo1[i].x = 1200; // Posición inicial aleatoria
        enemigo1[i].y = rand() % (750 - 300);
        enemigo1[i].ancho = 80;       // Ancho predeterminado
        enemigo1[i].alto = 80;        // Alto predeterminado
        enemigo1[i].activo = 0;       // Activo al inicio
        enemigo1[i].velocidad_x = 2; // Velocidad
        enemigo1[i].frameActual=0;
        enemigo1[i].tiempoFrame=200;
        enemigo1[i].tiempoAnterior=SDL_GetTicks();
        enemigo1[i].tiempoReaparicion=enemigo1[i].tiempoAnterior + TIEMPO_REAPARICION_ENEMIGO ; 
        enemigo1[i].vida = 2;     
    }
}

void actualizar_cronometro(Oleada *oleada_actual, Uint32 delta_tiempo) {
    if (!oleada_actual->en_transicion) {
        if (oleada_actual->tiempo_restante > delta_tiempo) {
            oleada_actual->tiempo_restante -= delta_tiempo; // Resta el tiempo transcurrido
        } else {
            oleada_actual->tiempo_restante = 0; // Evitar valores negativos
        }
    }
}

void generar_moneda(Moneda monedas[MAX_MONEDAS], int x, int y) {
    for ( i = 0; i < MAX_MONEDAS; i++) {
        if (!monedas[i].activo) {
            monedas[i].activo = 1;
            monedas[i].x = x;
            monedas[i].y = y;
            monedas[i].ancho = 10;
            monedas[i].alto = 10;
            break;
        }
    }
}

void actualizar_enemigo1 (Enemigo1 enemigo1[], Uint32 tiempo_actual, Oleada *oleada_actual){
	int i =0;
	for (i=0; i<MAX_ENEMIGOS; i++){
		if (enemigo1[i].activo){
			enemigo1[i].x -= enemigo1[i].velocidad_x;
			if (enemigo1[i].x < 0) {
				enemigo1[i].activo = 0;
				enemigo1[i].tiempoReaparicion = tiempo_actual + TIEMPO_REAPARICION_ENEMIGO;
			}
			if (tiempo_actual > enemigo1[i].tiempoAnterior + enemigo1[i].tiempoFrame) {
                enemigo1[i].frameActual = (enemigo1[i].frameActual + 1) % FRAMES_ENEMIGO1;
                enemigo1[i].tiempoAnterior = tiempo_actual;
			}
		}else if ((tiempo_actual > enemigo1[i].tiempoReaparicion) && (oleada_actual->tiempo_restante > 0)) {
		//si no esta activo, pregunta por condiciones de reaparicion y lo ejecuta si cumple, si el cronometro termino, dejo de spawnear enemigos
            	enemigo1[i].activo = 1;
            	enemigo1[i].vida = 3;
            	enemigo1[i].x = 1200; 
        		enemigo1[i].y = rand() % (750 - 300);
        		oleada_actual->enemigos_activos++;
		}
		
	}	
}

// Función para manejar daño al jugador
void recibir_dano(Jugador *player, int *game_over) {
        if (player->salud > 0) {
            player->salud--;
            printf("¡Jugador recibió daño! Vida restante: %d\n", player->salud);
        }
        if (player->salud <= 0) {
            printf("¡Juego terminado! Vida agotada.\n");
            *game_over = 1; // Activar Game Over
        }
    }
    
 // Función para reiniciar el juego
void reiniciar_juego(Jugador *player, Disparo disparos[], int *game_over,Enemigo1 enemigo1[], Enemigo2 *enemigo2, Oleada *oleada_actual) {
        int i = 0;
		player->x = 640;
        player->y = 300;
        player->dinero = 0;
        player->salud = VIDA_DEFAULT;
        for (i = 0; i < MAX_DISPAROS; i++) {
            disparos[i].activo = 0;
        }
        *game_over = 0;
        
        for (i = 0; i < MAX_ENEMIGOS; i++) {
        enemigo1[i].x = 1200; // Posición inicial aleatoria
        enemigo1[i].y = rand() % (750 - 300);
        enemigo1[i].ancho = 80;       // Ancho predeterminado
        enemigo1[i].alto = 80;        // Alto predeterminado
        enemigo1[i].activo = 0;       // Activo al inicio
        enemigo1[i].velocidad_x = 2; // Velocidad
        enemigo1[i].frameActual=0;
        enemigo1[i].tiempoFrame=200;
        enemigo1[i].tiempoAnterior=SDL_GetTicks();
        enemigo1[i].tiempoReaparicion=enemigo1[i].tiempoAnterior+TIEMPO_REAPARICION_ENEMIGO; 
        enemigo1[i].vida = 2;
    	}
    	
    	oleada_actual->momento_dia = 1;
    	oleada_actual->numero_oleada=0;
    	oleada_actual->oleada_maxima = (rand() % 5) + 1; //posibles oleadas entre 1 y 5
    	iniciar_oleada(oleada_actual,enemigo1); //inicializo oleada
    	
    	
    	enemigo2->x = 1000;
    	enemigo2->y = 300;
    	enemigo2->ancho = 80;
    	enemigo2->alto = 80;
    	enemigo2->activo = 1;
    	enemigo2->velocidad_x = 3;
    	enemigo2->frameActual = 0;
    	enemigo2->tiempoFrame = 200;
    	enemigo2->tiempoAnterior = SDL_GetTicks();
    	enemigo2->tiempoReaparicion = 0;
    	
        printf("Juego reiniciado.\n");
}

// Verificar fin de oleada
int verificar_fin_oleada(Oleada *oleada_actual, Enemigo1 enemigo1[]) {
    if (oleada_actual->tiempo_restante == 0) {
        for (i = 0; i < MAX_ENEMIGOS; i++) {
            if (enemigo1[i].activo) {
                return 0; // Aún hay enemigos activos
            }
        }
        return 1; // Todos los enemigos están inactivos
    }
    return 0; // Tiempo restante
}

// manejo de la transicion o momento entre oleadas y/o estado del dia
void manejar_transicion(Oleada *oleada_actual, Uint32 delta_tiempo,Enemigo1 enemigo1[]) {
    
	if (oleada_actual->tiempo_transicion > delta_tiempo) {// si entra aca, estoy esperando 15 segundos de descanso
        oleada_actual->tiempo_transicion -= delta_tiempo;
        printf("Transición activa. Tiempo restante: %d ms\n", oleada_actual->tiempo_transicion);
    } else {
        printf("Transición terminada. Iniciando siguiente oleada.\n");
        oleada_actual->en_transicion = 0;

        if (oleada_actual->numero_oleada < oleada_actual->oleada_maxima) {        	
        	iniciar_oleada(oleada_actual,enemigo1);
        } else {// o paso dia->atardecer | atardecer->noche | noche->amanecer | amanecer->dia 
        	if (oleada_actual->momento_dia == 4){ // si termine el tiempo de espera del amanecer pasare al momento del dia
				oleada_actual->momento_dia=1;
			}else {
			oleada_actual->momento_dia ++;
			}
          	printf("Cambio de momento del día: %d\n", oleada_actual->momento_dia);
          	if (oleada_actual->momento_dia == 2 || oleada_actual->momento_dia == 4){
          		oleada_actual->tiempo_transicion=15000;
          		oleada_actual->en_transicion=1;
			  }else{
			  	oleada_actual->numero_oleada=0;
    			oleada_actual->oleada_maxima = (rand() % 5) + 1; //posibles oleadas entre 1 y 5
    			iniciar_oleada(oleada_actual,enemigo1); //inicializo oleada
    			oleada_actual->en_transicion=0;
			  }
        	
        }
    }
}

void renderizar_texto(SDL_Renderer *renderer, TTF_Font *fuente, const char *texto, int x, int y) {
    // color del texto
    SDL_Color color = {255, 255, 255, 255}; // Blanco

    // Renderizar una superficie
    SDL_Surface *superficie_texto = TTF_RenderText_Solid(fuente, texto, color);
    if (!superficie_texto) {
        printf("Error al renderizar texto: %s\n", TTF_GetError());
        return;
    }

    // textura
    SDL_Texture *textura_texto = SDL_CreateTextureFromSurface(renderer, superficie_texto);
    SDL_Rect destino = {x, y, superficie_texto->w, superficie_texto->h};

    // Renderizar 
    SDL_RenderCopy(renderer, textura_texto, NULL, &destino);

    // Liberar recursos
    SDL_FreeSurface(superficie_texto);
    SDL_DestroyTexture(textura_texto);
}

void mostrar_oleadas(SDL_Renderer *renderer, TTF_Font *fuente, int numero_oleada, int total_oleadas) {
    char texto[32];
    sprintf(texto, "Oleada %d/%d", numero_oleada, total_oleadas);

    // Renderizar texto en pantalla
    renderizar_texto(renderer, fuente, texto, 1120, 5); // Ajusta la posición (50, 50)
}

void mostrar_cronometro(SDL_Renderer *renderer, TTF_Font *fuente, Oleada *oleada_actual) {
    char texto[64]; 
    Uint32 tiempo_mostrar;
    SDL_Color color = {255, 255, 255, 255}; // Blanco

    // mostrar el  cronometro adecuado
    if (oleada_actual->en_transicion) {
        tiempo_mostrar = oleada_actual->tiempo_transicion;
        snprintf(texto, sizeof(texto), "Descanzo: %d seg", tiempo_mostrar / 1000);
    } else {
        tiempo_mostrar = oleada_actual->tiempo_restante;
        snprintf(texto, sizeof(texto), "Oleada: %d seg", tiempo_mostrar / 1000);
    }

    // renderizar texto
    SDL_Surface *superficie_texto = TTF_RenderText_Solid(fuente, texto, color);
    SDL_Texture *textura_texto = SDL_CreateTextureFromSurface(renderer, superficie_texto);

    SDL_Rect destino = {640, 5, superficie_texto->w, superficie_texto->h};
    SDL_RenderCopy(renderer, textura_texto, NULL, &destino);

    // Liberar recursos
    SDL_FreeSurface(superficie_texto);
    SDL_DestroyTexture(textura_texto);
}

void inicializar_defensa(Defensa *barricada) {
    barricada->x = 100;           // Posición inicial
    barricada->y = 225;           // Fila donde estará la defensa
    barricada->ancho = 40;       // Ancho de la defensa
    barricada->alto = 370;         // Alto de la defensa
    barricada->construida = 0;    // Inicia como destruida
    barricada->vida = 0;
}

void renderizar_defensa(SDL_Renderer *renderer, Defensa *barricada, SDL_Texture *textura_construida, SDL_Texture *textura_destruida) {
    SDL_Texture *textura_actual = barricada->construida 
                                  ? textura_construida 
                                  : textura_destruida;
    SDL_Rect rect = {barricada->x, barricada->y, barricada->ancho, barricada->alto};
    SDL_RenderCopy(renderer, textura_actual, NULL, &rect);

}

void reconstruir_defensa(Defensa *barricada) {

    if (barricada->construida<1){
    	barricada->construida = 1; // la construyo
	}else{
		barricada->construida = 0; // la destruyo
	}
}



int main(int argc, char *argv[]) {
	srand(time(NULL));  // Inicializar el generador de números aleatorios
    // Inicializar SDL y SDL_image
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("No se pudo inicializar SDL: %s\n", SDL_GetError());
        return 1;
    }
    
    //inicializar SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("No se pudo inicializar SDL_image: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }
    
    //inicializar SDL_ttf
    if (TTF_Init() == -1) {
        printf("Error inicializando SDL_ttf: %s\n", TTF_GetError());
        return -1;
    }

    // Crear una ventana, 1280 para notebook, 1024 normal, 800 peque, el alto 600 esta al limite mas o menos
    //los parametros de createwindow son (nombre de ventana, posicion x, posicion y, ancho de ventana, alto de ventana, mostrar al crear)
    // asigno al tipo de variable SDL visual que reprecenta una ventana, bajo el nombre ventana
    
    SDL_Window *ventana = SDL_CreateWindow("Survival",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,1280, 600,SDL_WINDOW_SHOWN);
    //tratamiento de exepcion
	if (!ventana){
        printf("No se pudo crear la ventana: %s\n", SDL_GetError());
        TTF_Quit();
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
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    
    //carga de la fuente de letra
    TTF_Font *fuente = TTF_OpenFont("fuente/SHPinscher-Regular.otf", 24);
    
    if (!fuente) {
        printf("Error cargando fuente: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(ventana);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }
    
    // Color del texto (dorado)
    SDL_Color colorTexto = {255, 215, 0, 255};

	// Cargar texturas de los personajes, enemigos y proyectiles
    SDL_Texture *jugadorTextura = IMG_LoadTexture(renderer, "sprites/personaje2.png");
    if (!jugadorTextura) {
        printf("No se pudo cargar la textura de jugador: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(ventana);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_Texture *enemigo1Textura = IMG_LoadTexture(renderer, "sprites/enemigo1.png");
    if (!enemigo1Textura) {
        printf("No se pudo cargar la textura de enemigo1: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(ventana);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    SDL_Texture *enemigo2Textura = IMG_LoadTexture(renderer, "sprites/enemigo2.png");
    if (!enemigo2Textura) {
        printf("No se pudo cargar la textura de enemigo2: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(ventana);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
   
    SDL_Texture *proyectilEnemigo2Textura = IMG_LoadTexture(renderer, "sprites/proyectilEnemigo2.png");
    if (!proyectilEnemigo2Textura) {
        printf("No se pudo cargar la textura de proyectil enemigo: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(ventana);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    SDL_Texture *balaTextura = IMG_LoadTexture(renderer, "sprites/bala.png");
    if (!balaTextura) {
        printf("No se pudo cargar la textura de proyectil jugador: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(ventana);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
	//img de fondo
    SDL_Texture *fondoTextura = IMG_LoadTexture(renderer, "sprites/fondo.png");
    if (!fondoTextura) {
        printf("No se pudo cargar la textura de fondo: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(ventana);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    SDL_Texture *textura_defensa_construida = IMG_LoadTexture(renderer, "sprites/barricada2.png");
    if (!textura_defensa_construida) {
        printf("No se pudo cargar la textura de barricada construida: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(ventana);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
	SDL_Texture *textura_defensa_destruida = IMG_LoadTexture(renderer, "sprites/destruido.png");
    if (!textura_defensa_destruida) {
        printf("No se pudo cargar la textura de barricada destruida: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(ventana);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    //inicializacion de registro o entidad jugador y demas entidades necesarias para la logica
	Jugador player = {640, 300, 80, 80, VIDA_DEFAULT, 0, 5, 1, 0}; //pos x, pos y, ancho, alto, salud, dinero, velocidad, x ant, y ant, dinero
  	SDL_Rect jugador = {player.x, player.y, player.ancho, player.alto};
	 // Inicializar los disparos
    Disparo disparos[MAX_DISPAROS];
    Moneda monedas[MAX_MONEDAS];
    Enemigo1 enemigo1[MAX_ENEMIGOS];
    Oleada oleada_actual; // creo variable registro
    oleada_actual.numero_oleada=0;
    oleada_actual.momento_dia = 1;
    oleada_actual.oleada_maxima = (rand() % 5) + 1; //posibles oleadas entre 1 y 5
    iniciar_oleada(&oleada_actual,enemigo1); //inicializo oleada
    Defensa barricada;
    inicializar_defensa(&barricada);
    
    int i= 0;
    for ( i = 0; i < MAX_DISPAROS; i++) {
        disparos[i].activo = 0;
    }
    
    for (i = 0; i < MAX_DISPAROS; i++) {
        disparosEnemigo2[i].activo = 0;
	}
	
	for (i = 0; i < MAX_MONEDAS; i++){
		monedas[i].activo = 0;
	} 
	
    /* //lo siguiente es controlado al momento de la creacion de la entidad en el caso de la velocidad, el intervalo quedara en veremos

    // Variables para aparición continua de enemigos
    Uint32 tiempo_crear_enemigo = SDL_GetTicks();
    Uint32 intervalo_creacion = 2000; // Aparecen cada 2 segundos
    int velocidad_enemigo = 2;

    */
    
     // Variables para el cooldown de disparo
    Uint32 ultimo_disparo = 0;
    Uint32 cooldown = 500; // 500 milisegundos
    Enemigo2 enemigo2 = {1000, 300, 80, 80, 1, 3, 0, 200, SDL_GetTicks(), 0};
    int game_over = 0; //nesesario para logica de restart 
    int frameActual = 0;
    int tiempoFrame = 100;
    Uint32 tiempoAnterior = SDL_GetTicks();
    int enMovimiento = 0;
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    Animacion animacionActual = REPOSO;
    Uint32 tiempoInicioDisparo = 0;
    int duracionAnimacionDisparo = 400;
	Uint32 tiempo_anterior = SDL_GetTicks();
	
   // Bucle de eventos para mantener la ventana abierta
   
    int running = 1; //variable identificatoria para realizar bucle de muestreo/actualizacion hasta que el usuario intervenga
	SDL_Event evento;//asigno un nombre a los tipos de eventos que reacciona SDL_event, alli pararían todos los eventos por raton o teclado recolectados
    // los eventos pueden ser de raton, teclado o joystick, todos tienen palabras reservadas down y up, referenciando la accion de apretar y soltar respectivamente
    // se debe configurar ambos de lo contrario la accion se volvera perpetua
	int velocidad = 5; //velocidad traducida en 5 pixeles por bucle segun x accion de desplazamiento en cada eje
	Disparo disparo = {0, 0, 6, 2, 0, 0, 0}; // inicializamos la entidad disparo desactivada, los parametros son correspondientes y posicionales al struct
   
    while (running) {
        // Procesar eventos
        // SDL_PollEvent es algo asi como un CACHE de eventos captados por la "ventana" renderizada por SDL
        enMovimiento = 0;
        tiempo_actual = SDL_GetTicks();
        Uint32 delta_tiempo = tiempo_actual - tiempo_anterior;
    	tiempo_anterior = tiempo_actual;
        
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
                	
					case SDLK_k:// CURAR VIDA POR PROPOSITO DE DEPURACION Y EVOLUCION
                		if (player.salud < VIDA_DEFAULT) {
						        player.salud++;
						        printf("¡Jugador se curó! Vida actual: %d\n", player.salud);
						    } else {
						    	// OJO SI SUBES LA CANTIDAD DE VIDA MAXIMA SERA NESCESARIO MAS LOGICA EN ESTE PUNTO; O MEJO DICHO EN TODA ESTA ESTRUCTURA
						        printf("¡Vida al máximo! No se puede curar más.\n");
						    }
						    break;
					
					case SDLK_l: //realizarce daño para proposito de depuracion
						recibir_dano(&player,&game_over);
						break;
					
					case SDLK_m://depuracion de la defensa/barricada
						reconstruir_defensa(&barricada);
						break;
                 
                }
            }else if (evento.type == SDL_MOUSEBUTTONDOWN && game_over){
            	int mouse_x, mouse_y;
                SDL_GetMouseState(&mouse_x, &mouse_y);

                int menu_x = 540, menu_y = 200;
                int boton_ancho = 200, boton_alto = 60;

                SDL_Rect boton_reintentar = {menu_x, menu_y, boton_ancho, boton_alto};
                SDL_Rect boton_cerrar = {menu_x, menu_y + 80, boton_ancho, boton_alto};
                
                // logica para cerrar y reiniciar la app segun ubicacion y accion de el raton
                
                if (SDL_PointInRect(&(SDL_Point){mouse_x, mouse_y}, &boton_reintentar)) {
                    reiniciar_juego(&player, disparos, &game_over, enemigo1,&enemigo2,&oleada_actual);
                }
                
				if (SDL_PointInRect(&(SDL_Point){mouse_x, mouse_y}, &boton_cerrar)) {
                    running = 0;
                }
			}
        }
        //termina adm eventos
        
        //logica del juego, act de enemigos y disparos
        
        if (oleada_actual.en_transicion) {// tiempo para recoger monedas en caso de no llegar a la oleada final, si se llego hacer cambio de fondos
            manejar_transicion(&oleada_actual, delta_tiempo, enemigo1); 
        }else { // si no es una transicion o intermedio, estas en una oleada
            actualizar_cronometro(&oleada_actual, delta_tiempo);
            actualizar_enemigo1(enemigo1,tiempo_actual,&oleada_actual);
            
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
			
			// comprobacion de colicion entre disparos y enemigos
			for (i = 0; i < MAX_DISPAROS; i++) {
            	if (disparos[i].activo) {
	                SDL_Rect rect_disparo = {disparos[i].x, disparos[i].y, disparos[i].ancho, disparos[i].alto};
    	            SDL_RenderCopy(renderer, balaTextura, NULL, &rect_disparo);
					//comprobacion de interseccion entre enemigos tipo 1 y la bala
					for (j=0; j<MAX_ENEMIGOS;j++){
						if (enemigo1[j].activo) {
							SDL_Rect enemigo1Rect = {enemigo1[j].x + 15, enemigo1[j].y + 20, 40, 40};
							if (SDL_HasIntersection(&rect_disparo,&enemigo1Rect)){
								enemigo1[j].vida-=1;
								disparos[i].activo = 0;
								if (enemigo1[j].vida <= 0){
									enemigo1[j].activo = 0;
                    				enemigo1[j].tiempoReaparicion = tiempo_actual + TIEMPO_REAPARICION_ENEMIGO;
                    				// Generar moneda en la posición del enemigo muerto
									generar_moneda(monedas, enemigo1[j].x, enemigo1[j].y);	
									oleada_actual.enemigos_activos--;
								}
							}
						}
					}
                				
					//comprobacion de colivion entre enemigos tipo 2 y la bala
				
	                if (enemigo2.activo) {
						SDL_Rect enemigo2Rect = {enemigo2.x+15 , enemigo2.y + 20, 40, 40};
						if (SDL_HasIntersection(&rect_disparo,&enemigo2Rect)){
							disparos[i].activo = 0;
                	    	enemigo2.activo = 0;
                    		enemigo2.tiempoReaparicion = tiempo_actual + TIEMPO_REAPARICION_ENEMIGO;								
						}
					}
            	}
        	}
			
			// aqui iran las acciones de interseccion entre enemigos contra el jugador
				
			SDL_Rect jugadorHitbox = {player.x + 10, player.y + 20, 40, 40};
			
			for (i=0;i<MAX_ENEMIGOS;i++){// interseccion con enemigo 1
				if (enemigo1[i].activo){
					SDL_Rect enemigo1Rect = {enemigo1[i].x + 15, enemigo1[i].y + 20, 40, 40};
					if (SDL_HasIntersection(&jugadorHitbox,&enemigo1Rect)){
						//ejecuto acciones de DAÑO
						mostrarVentanaEmergente();
					}
				}
			}
			
			if (enemigo2.activo){ //interseccion con enemigo 2
				SDL_Rect enemigo2Rect = {enemigo2.x + 15, enemigo2.y + 20, 40, 40};
				if (SDL_HasIntersection (&jugadorHitbox, &enemigo2Rect)){
					mostrarVentanaEmergente();			
				}
			}	

		
            if (verificar_fin_oleada(&oleada_actual, enemigo1)) { //devuelve 1 si y solo si enemigos restantes = 0 y termino los 30 segundos de oleada
            // estoy en el fin de la oleada final, para evitar el doble tiempo de espera entre oleada y cambio de momento del dia se coloca en 0
                oleada_actual.en_transicion = 1;
                oleada_actual.tiempo_transicion = 0; // 15 segundos
			}
        }
        // renderizados etc abajo
        // Control de animaciones del jugador
        if (animacionActual == DISPARO && SDL_GetTicks() > tiempoInicioDisparo + duracionAnimacionDisparo) {
            animacionActual = REPOSO;
        }

        // Actualizar frame de animación
        Uint32 tiempoActual = SDL_GetTicks();

        if (animacionActual == CAMINAR) {
		    // Ralentizamos la animación al caminar
		    if (tiempo_actual > tiempoAnterior + tiempoFrameCaminar) {
		        frameActual = (frameActual + 1) % FRAMES_CAMINAR;
		        tiempoAnterior = tiempo_actual;
		    }
		} else if (animacionActual == REPOSO) {
		    // Ralentizamos la animación de reposo
		    if (tiempo_actual > tiempoAnterior + tiempoFrameReposo) {
		        frameActual = (frameActual + 1) % FRAMES_REPOSO;
		        tiempoAnterior = tiempo_actual;
		    }
		} else {
		    // Para otras animaciones, puedes dejar el tiempo por defecto
		    if (tiempo_actual > tiempoAnterior + tiempoFrame) {
		        frameActual = (frameActual + 1) % FRAMES_REPOSO;
		        tiempoAnterior = tiempo_actual;
		    }
		}

        // Movimiento de disparos jugador
        for (i = 0; i < MAX_DISPAROS; i++) {
            if (disparos[i].activo) {
                disparos[i].x += disparos[i].direccion_x * VELOCIDAD_DISPARO;
                if (disparos[i].x < 0 || disparos[i].x > 1280) {
                    disparos[i].activo = 0;
                }
            }
        }
		/*
		
		ACA FALTAN VARIABLES PARA CADA UNO DE LOS FONDOS; MAS ARRIBA CERCA DE LAS INICIALIZACIONES
		
		// Renderizar fondo según el momento del día
    	if (oleada_actual.momento_dia == 1) {
	        SDL_RenderCopy(renderer, fondoDiaTextura, NULL, NULL);
	    } else if (oleada_actual.momento_dia == 2) {
	        SDL_RenderCopy(renderer, fondoAtardecerTextura, NULL, NULL);
	    } else if (oleada_actual.momento_dia == 3) {
	        SDL_RenderCopy(renderer, fondoNocheTextura, NULL, NULL);
	    } else if (oleada_actual.momento_dia == 4) {
	        SDL_RenderCopy(renderer, fondoAmanecerTextura, NULL, NULL);
	    }
		*/
		SDL_RenderCopy(renderer, fondoTextura, NULL, NULL);
	    
        // Actualización de posición del rectángulo de representación
		jugador.x = player.x;
		jugador.y = player.y;
		jugador.w = player.ancho;
		jugador.h = player.alto;
        
		// Renderizado del jugador
		
		SDL_Rect srcRect = {frameActual * ANCHO_FRAME, animacionActual * ALTO_FRAME, ANCHO_FRAME, ALTO_FRAME};
		SDL_Rect dstRect = {jugador.x, jugador.y, jugador.w, jugador.h};
		SDL_RenderCopyEx(renderer, jugadorTextura, &srcRect, &dstRect, 0, NULL, flip);
		 
        // renderizado visual de enemigo1 activo, funciones utilizadas para renderizar con textura asociada, en este caso el zombie
        for (i=0; i<MAX_ENEMIGOS ;i++){
        	if (enemigo1[i].activo) {
                SDL_Rect enemigo1SrcRect = {enemigo1[i].frameActual * ANCHO_FRAME, 0, ANCHO_FRAME, ALTO_FRAME};
                SDL_Rect enemigo1DstRect = {enemigo1[i].x, enemigo1[i].y, enemigo1[i].ancho, enemigo1[i].alto};
                SDL_RenderCopy(renderer, enemigo1Textura, &enemigo1SrcRect, &enemigo1DstRect);
            }
		}
		
		// renderizado para el mago y renderizados de su disparo si existe
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
		//colicion/ interseccion entre jugador y moneda
		SDL_Rect jugadorHitbox = {player.x + 10, player.y + 20, 40, 40};
		
        for ( i = 0; i < MAX_MONEDAS; i++) {
		    if (monedas[i].activo) {
		        SDL_Rect moneda_rect = {monedas[i].x, monedas[i].y, monedas[i].ancho, monedas[i].alto};
		        if (SDL_HasIntersection(&moneda_rect, &jugadorHitbox)) {
		            // El jugador recoge la moneda
		            monedas[i].activo = 0;  // Desactivar moneda
		           // puntuacion += 10;       // Aumentar puntuación (puedes cambiar el valor si deseas)
		            printf("se recogio la moneda");  // Mostrar puntuación en consola
		            player.dinero++;
		        }
		    }
		}
  		
  		// Renderizar monedas
		SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255); // Color dorado para las monedas
		for ( i = 0; i < MAX_MONEDAS; i++) {
		    if (monedas[i].activo) {
		        SDL_Rect rect = {monedas[i].x, monedas[i].y, monedas[i].ancho, monedas[i].alto};
		        SDL_RenderFillRect(renderer, &rect);
		    }
		}
		
		// renderizado de Indicador de vida
        for (i = 0; i < player.salud; i++) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_Rect vida_rect = {10 + i * 30, 10, 20, 20};
            if (i >= player.salud) {
                SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
            }
            SDL_RenderFillRect(renderer, &vida_rect);
        }
  		
  		// Mostrar menú de Game Over
        if (game_over) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_Rect boton_reintentar = {540, 200, 200, 60};
            SDL_Rect boton_cerrar = {540, 280, 200, 60};
            SDL_RenderDrawRect(renderer, &boton_reintentar);
            SDL_RenderDrawRect(renderer, &boton_cerrar);
        }
  		
  		// mostar dinero con sdl_ttf
  		// se define y convierte a caracter o string lo que se quiera mostrar OBLIGADAMENTE
        char texto[50];
        snprintf(texto, sizeof(texto), "Dinero: %d", player.dinero);
        
         // Renderizar el texto dinámico
        SDL_Surface* superficieTexto = TTF_RenderText_Solid(fuente, texto, colorTexto);
        SDL_Texture* texturaTexto = SDL_CreateTextureFromSurface(renderer, superficieTexto);
        
  		// posición y tamaño del texto, como se asemeja al tratamiento de una textura, realizamos un rectangulo para "rellenar" con el texto y moverlo a donde queramos
        SDL_Rect rectTexto;
        rectTexto.x = 960;  // Coordenada X
        rectTexto.y = 5;  // Coordenada Y
        rectTexto.w = superficieTexto->w; // Ancho del texto
        rectTexto.h = superficieTexto->h; // Alto del texto
        SDL_FreeSurface(superficieTexto); // Liberar superficie después de crear la textura
        // Dibujar el texto
        SDL_RenderCopy(renderer, texturaTexto, NULL, &rectTexto);
        SDL_DestroyTexture(texturaTexto);
    
        mostrar_oleadas(renderer, fuente, oleada_actual.numero_oleada, oleada_actual.oleada_maxima);
  		
  		mostrar_cronometro(renderer, fuente, &oleada_actual);
  		
  		renderizar_defensa(renderer, &barricada, textura_defensa_construida, textura_defensa_destruida);

  		
		SDL_RenderPresent(renderer);
        SDL_Delay(16);
	}
		
    // Liberar recursos y cerrar SDL
    //SDL_DestroyTexture(proyectilEnemigo2Textura);
    if (proyectilEnemigo2Textura != NULL) { 
		SDL_DestroyTexture(proyectilEnemigo2Textura); 
	} else { 
        printf("proyectilEnemigo2Textura no fue inicializado correctamente\n");
    }
	SDL_DestroyTexture(fondoTextura);
    SDL_DestroyTexture(balaTextura);
    SDL_DestroyTexture(enemigo1Textura);
    SDL_DestroyTexture(enemigo2Textura);
    SDL_DestroyTexture(jugadorTextura);
    SDL_DestroyTexture(textura_defensa_construida);
	SDL_DestroyTexture(textura_defensa_destruida);
    TTF_CloseFont(fuente);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(ventana);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
