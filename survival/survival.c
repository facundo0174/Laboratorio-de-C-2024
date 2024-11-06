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

#define MAX_DISPAROS 10

int main(int argc, char *argv[]) {
	
    // Inicializar SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("No se pudo inicializar SDL: %s\n", SDL_GetError());
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

	  // Establecer el color de fondo (azul)
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // RGB: azul, aplicamos azul a la entidad
    SDL_RenderClear(renderer); // Llenar con el color establecido, borramos todo lo que tenia la entidad y aplica cambios AUN NO VISIBLES
    SDL_RenderPresent(renderer); // Mostrar en pantalla LOS CAMBIOS NO VISIBLES
    
    SDL_Rect rectangulo = {0, 100, 1280, 400}; // realiza un rectangulo relleno, paramentros = (coordenadas x, coordenads y, largo, alto)
    //SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);//este color se APLICARA a EL SIGUIENTE DIBUJO, si no aparece nada se aplica al FONDO o VENTANA
    //SDL_RenderFillRect(renderer, &rectangulo);// crea el dibujo RECTANGULO RELLENO, dentro de renderer con la direccion de rectangulo PERO AUN NO ES VISIBLE
    //SDL_RenderPresent(renderer);//HACE VISIBLE LOS CAMBIOS ANTERIORES
  	
  	SDL_Rect jugador = {640, 300, 20, 20};
	 // Inicializar los disparos
    Disparo disparos[MAX_DISPAROS];
    int i= 0;
    for ( i = 0; i < MAX_DISPAROS; i++) {
        disparos[i].activo = 0;
    }
     // Variables para el cooldown de disparo
    Uint32 ultimo_disparo = 0;
    Uint32 cooldown = 500; // 500 milisegundos

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
        
        while (SDL_PollEvent(&evento)) {
        	//recorre la cola de eventos y realiza opciones segun corresponda
        	if (evento.type == SDL_QUIT) {
                running = 0;
            } else if (evento.type == SDL_KEYDOWN) {
                switch (evento.key.keysym.sym) {
                    case SDLK_w: // Movimiento hacia arriba
                        if (jugador.y - velocidad >= rectangulo.y){
						 	jugador.y -= velocidad;
						 	ultima_direccion_x = 0; 
                        	ultima_direccion_y = -1;
                        }
						break;
                    case SDLK_s: // Movimiento hacia abajo
                        if (jugador.y + jugador.h + velocidad <= rectangulo.y + rectangulo.h){
                        	jugador.y += velocidad;
                        	ultima_direccion_x = 0; 
                            ultima_direccion_y = 1;
						} 
                        break;
                    case SDLK_a: // Movimiento hacia la izquierda
                        if (jugador.x - velocidad >= rectangulo.x){
                        	 jugador.x -= velocidad;
                        	 ultima_direccion_x = -1; 
                             ultima_direccion_y = 0;
                    }
						break;
                    case SDLK_d: // Movimiento hacia la derecha
                        if (jugador.x + jugador.w + velocidad <= rectangulo.x + rectangulo.w){
							jugador.x += velocidad;
							ultima_direccion_x = 1; 
                            ultima_direccion_y = 0;
                        }
                        break;
                    case SDLK_j:{// disparo con la letra "j"
                            Uint32 tiempo_actual = SDL_GetTicks();
                            if (tiempo_actual > ultimo_disparo + cooldown) {
                                for (i = 0; i < MAX_DISPAROS; i++) {
                                    if (!disparos[i].activo) { // Encontrar un disparo inactivo
                                        disparos[i].activo = 1;
                                        disparos[i].x = jugador.x + jugador.w / 2 - 3; // Centrar el disparo
                                        disparos[i].y = jugador.y + jugador.h / 2 - 1;
                                        disparos[i].ancho = 6;
                                        disparos[i].alto = 2;
                                        disparos[i].direccion_x = ultima_direccion_x;
                                        disparos[i].direccion_y = ultima_direccion_y;
                                        ultimo_disparo = tiempo_actual;
                                        break; // Solo un disparo por pulsación
                                    }
                                }
                            }
                        }
                    	break;
                }
            }
        }
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
        }
           
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
           

    // Limpiar y cerrar SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(ventana);
    SDL_Quit();
    
    return 0;
}

