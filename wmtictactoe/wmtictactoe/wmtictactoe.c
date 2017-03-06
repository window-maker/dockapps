/*
   wmtictactoe - the ultimate tictactoe for WindowMaker 
   =-=-=-=-=-=   ====================================== 
                  Copyright (C) 1999 André R. Camargo

   Este programa é um software de livre distribuição, que pode ser copiado e 
   distribuído sob os termos da Licença Pública Geral GNU, conforme publicada 
   pela Free Software Foundation, versão 2 da licença ou (a critério do autor)
   qualquer versão posterior. 

   Este programa é distribuído na expectativa de ser útil aos seus usuários, 
   porém  NÃO TEM NENHUMA GARANTIA, EXPLÍCITAS OU IMPLÍCITAS, COMERCIAIS OU DE
   ATENDIMENTO A UMA DETERMINADA FINALIDADE. Consulte a Licença Pública Geral
   GNU para maiores detalhes. 

   Deve haver uma cópia da Licença Pública Geral GNU junto com este software 
   em inglês ou português. Caso não haja escreva para 
   Free Software Foundation, Inc.
   675 Mass Ave,
   Cambridge, MA 02139, USA. 

   acamargo@conesul.com.br
   André Ribeiro Camargo
   Rua Silveira Martins, 592/102
   Centro
   Canguçu-RS-Brasil
   CEP 96.600-000
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include <libdockapp/wmgeneral.h>
#include "wmtictactoe-master.xpm"

// ---------------------------------------------------------------
// definicoes :)

#define WMTICTACTOE_VERSION "1.1"

#define TRUE            1
#define FALSE           0

#define USLEEP          20000
#define BLINK           200000

#define LEGENDA_VAZIO   0
#define LEGENDA_USUARIO 1
#define LEGENDA_X       2

#define JOGO_OFENSIVO   0
#define JOGO_DEFENSIVO  1

// ---------------------------------------------------------------
// Variáveis Globais

char   *ProgName;

typedef struct {
	int     left;
	int     top;
	int     right;
	int     bottom;
} regioes;

regioes quadrantes[MAX_MOUSE_REGION] =
{
	{5, 8, 20, 18},
	{24, 8, 40, 18},
	{43, 8, 57, 18},
	{5, 20, 20, 30},
	{24, 20, 40, 30},
	{43, 20, 57, 30},
	{5, 33, 20, 44},
	{24, 33, 40, 44},
	{43, 33, 57, 44}};

int     tabuleiro[9] =
{LEGENDA_VAZIO, LEGENDA_VAZIO, LEGENDA_VAZIO,
 LEGENDA_VAZIO, LEGENDA_VAZIO, LEGENDA_VAZIO,
 LEGENDA_VAZIO, LEGENDA_VAZIO, LEGENDA_VAZIO};

int     sequencias[8][3] =
{
	{0, 1, 2},
	{3, 4, 5},
	{6, 7, 8},
	{0, 3, 6},
	{1, 4, 7},
	{2, 5, 8},
	{0, 4, 8},
	{2, 4, 6}};

int     livre[9] =
{0, 1, 2, 3, 4, 5, 6, 7, 8};
int     livre_max;
int     game_mode;
int     mute_mode;
int     score_user_offensive = 0;
int     score_X_offensive = 0;
int     score_deuce_offensive = 0;
int     score_user_defensive = 0;
int     score_X_defensive = 0;
int     score_deuce_defensive = 0;
int     score_opponent = 0;
// modo padrao eh jogar contra o micro
// por isso deadmatch leva um FALSE
int	isDeadmatch = FALSE;
int	adversario =  TRUE;

// mascara
char    wmtictactoe_mask_bits[64 * 64];
int     wmtictactoe_mask_width = 64;
int     wmtictactoe_mask_height = 64;

// ----------------------------------------------------------
// declaracao das funcoes do sistema

void    main (int argc, char *argv[]);
void    usage (void);
void    printversion (void);
void    readfile (void);
void    writefile (void);

void    desenhaJogador (int);
void    desenhaX (int);
void    desenhaAdversario (int);
void    desenhaLimpa (int);
void    desenhaAvisoJoga (void);
void    desenhaAvisoLimpa (void);
void    desenhaAvisoVoceVenceu (void);
void    desenhaAvisoEuVenci (void);
void    desenhaAvisoEmpate (void);

int     mostra_score (void);
void    escreve_placar (void);
void    reseta_score (void);
void    piscaVencedor (void);
void    troca_game_mode (void);

void    reseta_tabuleiro (void);
void    livre_desempilha (int);

void    principal (int, char **);
int     escolheJogador (void);

void    joga (int);
void    jogaHumano (int);

void    jogaX (void);
int     tapa_buraco (void);
int     analisa_jogo (void);
int     chuta_jogada (void);

int     validaJogada (int);
void    game_over (void);

// -------------------------------------------------------------------------------------------
//    funcao: main()
// descricao: funcao principal da linguagem
//        in: argc - numero de argumentos passados por linha d comando
//            argv - vetor com os argumentos
//       out: nada
void main (int argc, char *argv[])
{
	int     i;
     
	ProgName = argv[0];
	if (strlen (ProgName) >= 11)
		ProgName += (strlen (ProgName) - 11);

	game_mode = JOGO_DEFENSIVO;
	mute_mode = FALSE;
	
	for (i = 1; i < argc; i++) {
		char   *arg = argv[i];

		if (*arg == '-') {
			switch (arg[1]) {
				case 'o':
					game_mode = JOGO_OFENSIVO;
					break;
				case 'd':
					printf("%s", arg+1);
					if (strcmp (arg + 1, "deadmatch") == 0) {
				  		isDeadmatch = TRUE;
						break;
					}
		  			if (strcmp (arg + 1, "display") == 0) 
				  		break;
					usage ();
					exit (1);
				case 'v':
					printversion ();
					exit (0);
			        case 'q':
				        mute_mode = TRUE;
					break;
				default:
					usage ();
					exit (0);
			}
		}
	}

	if (mute_mode) {
	        fprintf (stderr, "\nwmTicTacToe %s - Copyright © 1999 André Ribeiro Camargo\n\n", WMTICTACTOE_VERSION);
		fprintf (stderr, "Este software NÃO POSSUI NENHUMA GARANTIA; Este é um ");
		fprintf (stderr, "software de livre \ndistribuição e você está autorizado a distribui-lo dentro de certas\n");
		fprintf (stderr, "condições. Verifique a documentação do sistema para maiores detalhes.\n");
		fprintf (stderr, "\n\n\"Thank you for shopping at Pop Mart\"-U2\n");

		fprintf (stderr, "\nPlaying on %s mode... %s", isDeadmatch ? "deadmatch" : (game_mode == JOGO_DEFENSIVO) ? "DEFENSIVE" : "OFFENSIVE", isDeadmatch ? ":) <-> (:" : (game_mode == JOGO_DEFENSIVO) ? ":(" : ":)");
	}
	
	principal (argc, argv);
}

// -------------------------------------------------------------------------------------------
//    funcao: desenhaAdversario(int quadrante)
// descricao: desenha a jogada feita pelo adversario no tabuleiro
//        in: quadrante - quadrante do tabuleiro
//       out: nada
void
desenhaAdversario (int quadrante)
{
	tabuleiro[quadrante] = LEGENDA_X;
	copyXPMArea(97, 74, 13, 8, quadrantes[quadrante].left + 1, quadrantes[quadrante].top + 1);
}

// -------------------------------------------------------------------------------------------
//    funcao: desenhaJogador(int quadrante)
// descricao: desenha o jogador no tabuleiro
//        in: quadrante - quadrante do tabuleiro
//       out: nada
void
desenhaJogador (int quadrante)
{
	tabuleiro[quadrante] = LEGENDA_USUARIO;
	copyXPMArea (68, 4, 13, 8, quadrantes[quadrante].left + 1, quadrantes[quadrante].top + 1);
}

// --------------------------------------------------------------------------------------------
//    funcao: desenhaX(int quadrante)
// descricao: desenha o X no tabuleiro
//        in: quadrante - quadrante do tabuleiro
//       out: nada
void
desenhaX (int quadrante)
{
	if (isDeadmatch)
	  desenhaAdversario(quadrante);
	else
	        if (game_mode == JOGO_DEFENSIVO)
  		        copyXPMArea (96, 4, 13, 8, quadrantes[quadrante].left + 1, quadrantes[quadrante].top + 1);
		else
		        copyXPMArea (110, 4, 13, 8, quadrantes[quadrante].left + 1, quadrantes[quadrante].top + 1);
}

// --------------------------------------------------------------------------------------------
//    funcao: desenhaLimpa(int quadrante)
// descricao: apaga o jogador que estiver no quadrante especificando
//        in: quadrante - quadrante do tabuleiro
//       out: nada
void
desenhaLimpa (int quadrante)
{
	copyXPMArea (82, 4, 13, 8, quadrantes[quadrante].left + 1, quadrantes[quadrante].top + 1);
}

// ---------------------------------------------------------------------------------------------
//    funcao: desenhaAvisoJoga
// descricao: desenha "play" na tela
//        in: nada
//       out: nada
void
desenhaAvisoJoga (void)
{
	if (isDeadmatch && adversario) {
		copyXPMArea(68, 103, 60, 9, 4, 47);
		copyXPMArea(71, 93, 16, 8, 5, 47);
		return;
	}
	if (isDeadmatch && !adversario) {
		copyXPMArea(68, 103, 60, 9, 4, 47);
		return;

	}
	copyXPMArea (68, 13, 60, 9, 4, 47);
}

// ----------------------------------------------------------------------------------------------
//    funcao: desenhaAvisoVoceVenceu
// descricao: desenha "you win" na tela
//        in: nada
//       out: nada
void
desenhaAvisoVoceVenceu (void)
{
	if (isDeadmatch)
		copyXPMArea (68, 83, 60, 9, 4, 47);
	else
		copyXPMArea (68, 23, 60, 9, 4, 47);
}

// -----------------------------------------------------------------------------------------------
//    funcao: desenhaAvisoEuVenci
// descricao: desenha "I win" na tela
//        in: nada
//       out: nada
void
desenhaAvisoEuVenci (void)
{
	if (isDeadmatch)
		copyXPMArea (68, 93, 60, 9, 4, 47);
	else
		copyXPMArea (68, 33, 60, 9, 4, 47);
}

// -----------------------------------------------------------------------------------------------
//    funcao: desenhaAvisoEmpate
// descricao: desenha "deuce" na tela
//        in: nada
//       out: nada
void
desenhaAvisoEmpate (void)
{
	copyXPMArea (68, 43, 60, 9, 4, 47);
}

// -----------------------------------------------------------------------------------------------
//    funcao: desenhaAvisoLimpa
// descricao: "apaga" os displays da tela
//        in: nada
//       out: nada
void
desenhaAvisoLimpa (void)
{
	copyXPMArea (68, 53, 60, 9, 4, 47);
}

// ------------------------------------------------------------------------------------------------
//    funcao: validaJogada
// descricao: Verifica se o quadrante jah nao estah ocupado
//        in: quadrante
//       out: 0 - quadrante ocupado
//            1 - quadrante disponivel, jogada valida!
int
validaJogada (int quadrante)
{
	return ((quadrante > -1) ? tabuleiro[quadrante] == 0 : 0);
}

// ------------------------------------------------------------------------------------------------
//    funcao: reseta_tabuleiro
// descricao: reseta o jogo
//        in: nada
//       out: nada
void
reseta_tabuleiro (void)
{
	int     i;

	for (i = 0; i < 9; i++) {
		tabuleiro[i] = LEGENDA_VAZIO;
		desenhaLimpa (i);
		livre[i] = i;
	}
	livre_max = i;

	desenhaAvisoJoga ();
}

// ------------------------------------------------------------------------------------------------
//    funcao: escolheJogador
// descricao: escolhe qual dos jogadores iniciarah a partida
//        in: nada
//       out: 0 - comeca pelo X
//            1 - comeca pelo Usuario
int
escolheJogador (void)
{
	srand ((int) time (NULL));
	adversario = ((int) ((float) random () / (float) RAND_MAX * 2));
	return adversario;
}

// ------------------------------------------------------------------------------------------------
//    funcao: verificaSequencia
// descricao: verifica se foi fechada alguma sequencia
//        in: nada
//       out: -2 - todos os quadrantes preenchido e nenhuma sequencia fechada, ou seja, empate!
//            -1 - nao foi encontrado sequencia fechada
//            > -1 - numero da sequencia fechada
int
verificaSequencia ()
{
	/*
	   matriz d jogadas contem o numero da posicao

 	    0 | 1 | 2
	   ---+---+---
	    3 | 4 | 5
	   ---+---+---
	    6 | 7 | 8
	 */

	int     sucesso = 0;
	int     padrao;
	int     i;

	for (i = 0; i < 8; i++) {
		padrao = tabuleiro[sequencias[i][0]];
		if (padrao == LEGENDA_VAZIO)
			continue;
		sucesso = ((tabuleiro[sequencias[i][1]] == padrao) &&
			   (tabuleiro[sequencias[i][2]] == padrao));
		if (sucesso)
			break;
	}

	// verifica se ha algum quadrante para se jogar
	if (!sucesso && !livre_max)
		return (-2);

	return ((sucesso) ? i : -1);
}

// ------------------------------------------------------------------------------------
//    funcao: game_over
// descricao: caso o jogo tenha acabado, pisca vencedor
//        in: nada
//       out: nada
void
game_over ()
{
	if (verificaSequencia () != -1)
		piscaVencedor ();
}

// -------------------------------------------------------------------------------------
//    funcao: piscaVencedor
// descricao: pisca as jogadas da sequencia especifica vencedora 
//        in: nada
//       out: nada
void
piscaVencedor ()
{
	int     mostra = 0;
	int     i;
	int     seq = verificaSequencia ();
	int     jogador = tabuleiro[sequencias[seq][0]];
	XEvent  Event;

	// incrementa o score do vencedor
	if (seq == -2) {
                if (game_mode == JOGO_OFENSIVO)
		        (score_deuce_offensive > 98) ? score_deuce_offensive = 1 : score_deuce_offensive++;
                else
	                (score_deuce_defensive > 98) ? score_deuce_defensive = 1 : score_deuce_defensive++;
	} else
                if (jogador == LEGENDA_X) {
                        if (game_mode == JOGO_OFENSIVO)
		                (score_X_offensive > 98) ? score_X_offensive = 1 : score_X_offensive++;
                        else
                                (score_X_defensive > 98) ? score_X_defensive = 1 : score_X_defensive++;
	        } else {
                        if (game_mode == JOGO_OFENSIVO)
                                (score_user_offensive > 98) ? score_user_offensive = 1 : score_user_offensive++;
                        else
                                (score_user_defensive > 98) ? score_user_defensive = 1 : score_user_defensive++;
		}

	if (!isDeadmatch)
		writefile ();

	while (1) {
		RedrawWindow ();

		usleep (BLINK);
		while (XPending (display)) {
			XNextEvent (display, &Event);
			switch (Event.type) {
				case Expose:
					RedrawWindow ();
					break;
				case DestroyNotify:
					XCloseDisplay (display);
					exit (0);
					break;
				case ButtonRelease:
					switch (Event.xbutton.button) {
					        case 3:
						        if (mostra_score ())
                                                                return;
							break;
                                                default:
                                                        reseta_tabuleiro ();
                                                        return;
					}
			}

		}
		if (mostra) {
			if (seq == -2)
				desenhaAvisoEmpate ();
			else {
				if (jogador == LEGENDA_USUARIO)
					desenhaAvisoVoceVenceu ();
				else
					desenhaAvisoEuVenci ();
				for (i = 0; i < 3; i++)
					if (jogador == LEGENDA_USUARIO)
						desenhaJogador (sequencias[seq][i]);
					else
						desenhaX (sequencias[seq][i]);
			}
		} else {
			desenhaAvisoLimpa ();
			if (seq != -2)
				for (i = 0; i < 3; i++)
					desenhaLimpa (sequencias[seq][i]);
		}
		mostra = !mostra;
	}
}

// -------------------------------------------------------------------------------------
//    funcao: escreve_placar
// descricao: escreve o placar do jogo na tela de score
//        in: nada
//       out: nada
void
escreve_placar ()
{
        int i;
        int coluna_xpm = 65;
	int coluna_score[6] = 
	{8, 15, 26, 33, 43, 50 };
        char placar[6];

  	if (isDeadmatch){
    		copyXPMArea(97, 74, 13, 9, 43, 88);
		if (!mute_mode)
		  sprintf(placar, 
			  "%.2d%.2d%.2d", 
			  game_mode == JOGO_OFENSIVO ? score_user_offensive : score_user_defensive, 
			  game_mode == JOGO_OFENSIVO ? score_deuce_offensive : score_deuce_defensive,
			  game_mode == JOGO_OFENSIVO ? score_X_offensive : score_X_defensive);
	}
	else
	// desenha o glyph do X modo ofensivo no placar 
		if (game_mode == JOGO_OFENSIVO) {
		        copyXPMArea (110, 4, 13, 8, 43, 88);
                	if (!mute_mode)
		        	sprintf(placar, "%.2d%.2d%.2d", score_user_offensive, score_deuce_offensive, score_X_offensive);
	        } else {
        	        copyXPMArea (96, 4, 13, 8, 43, 88);
			if (!mute_mode)
	                	sprintf(placar, "%.2d%.2d%.2d", score_user_defensive, score_deuce_defensive, score_X_defensive);
		}
	
	for (i = 0; i < 6; i++)
       		copyXPMArea (coluna_xpm+((placar[i]-48)*6), 65, 6, 9, coluna_score[i], 100);
}

// -------------------------------------------------------------------------------------
//    funcao: reseta_score
// descricao: zera o placar do jogo
//        in: nada
//       out: nada
void
reseta_score ()
{
        score_X_offensive = 0;
	score_user_offensive = 0;
	score_deuce_offensive = 0;
	score_X_defensive = 0;
        score_user_defensive = 0;
        score_deuce_defensive = 0;
	score_opponent = 0;

	writefile ();

	escreve_placar ();
}

// -------------------------------------------------------------------------------------
//    funcao: mostra_score
// descricao: mostra o placar e aguarda o usuario pressionar qq botao
//            para voltar ao jogo
//        in: nada
//       out: 0: se o modo d jogo continua o mesmo
//            1: se o modo d jogo foi alterado
int
mostra_score ()
{
	XEvent  Event;
        int     game_mode_changed = 0;

        escreve_placar ();
	while (1) {
                RedrawWindowXY (0, 60);

		while (XPending (display)) {
			XNextEvent (display, &Event);
			switch (Event.type) {
				case Expose:
					RedrawWindow ();
					break;
				case DestroyNotify:
					XCloseDisplay (display);
					exit (0);
					break;
				case ButtonRelease:
				        if (Event.xbutton.button == 1 &&
					    !isDeadmatch) {
                                                troca_game_mode ();
                                                game_mode_changed = 1;
                                                escreve_placar ();
				        } else
                                                if (Event.xbutton.button == 2)
					                reseta_score ();
					        else
					                return (game_mode_changed);
			}

		}
		usleep (USLEEP);
	}
}

// ----------------------------------------------------------------------------------
//    funcao: principal
// descricao: funcao principal do jogo
//        in: argc - numero de argumentos passados por main()
//            argv - matriz de strings com os argumentos passador por main()
//       out: nada
void
principal (int argc, char **argv)
{
	int     i;
	XEvent  Event;

	createXBMfromXPM (wmtictactoe_mask_bits, wmtictactoe_master_xpm, wmtictactoe_mask_width, wmtictactoe_mask_height);
	openXwindow (argc, argv, wmtictactoe_master_xpm, wmtictactoe_mask_bits, wmtictactoe_mask_width, wmtictactoe_mask_height);

	for (i = 0; i < MAX_MOUSE_REGION; i++)
		AddMouseRegion (i, quadrantes[i].left, quadrantes[i].top, quadrantes[i].right, quadrantes[i].bottom);

	reseta_tabuleiro ();

	if (!isDeadmatch)
		readfile ();
	mostra_score ();

	if (!isDeadmatch && escolheJogador ())
		jogaX ();

	desenhaAvisoJoga ();

	while (1) {
	        RedrawWindow ();

		while (XPending (display)) {
			XNextEvent (display, &Event);
			switch (Event.type) {
				case Expose:
					RedrawWindow ();
					break;
				case DestroyNotify:
					XCloseDisplay (display);
					exit (0);
					break;
				case ButtonRelease:
					i = CheckMouseRegion (Event.xbutton.x, Event.xbutton.y);
					switch (Event.xbutton.button) {
						case 1:
							if (validaJogada (i)) {
								jogaHumano (i);
								if (isDeadmatch)
									desenhaAvisoJoga ();
								else
									jogaX ();
							}
							break;
						case 2:
							reseta_tabuleiro ();
							break;
					        case 3:
						        mostra_score ();
					}
			}

		}
		usleep (USLEEP);
	}

}

// ------------------------------------------------------------------------------
//    funcao: livre_desempilha
// descricao: esta rotina retira o quadrante "quad" da matriz de posicoes vazias
//        in: quadrante
//       out: nada
void
livre_desempilha (int quad)
{
	int     i = 0;

	// localiza quadrante no vetor de quadrantes livres
	while (livre[i] < quad)
		i++;

	// desempilha
	while (i < livre_max) {
		livre[i] = livre[i + 1];
		i++;
	}

	// seta o ultimo elemento como -1
	// *assim fica + facil debugar :) *
	livre[--livre_max] = -1;
}

// ------------------------------------------------------------------------------
//    funcao: tapa_buraco
// descricao: verifica se o usuario nao estah por fechar alguma sequencia,
//            retornando o quadrante onde o X deverah jogar para anular a jogada
//        in: nada
//       out: -1 - nao ha buraco
//            > -1 - quadrante que tapa
int
tapa_buraco (void)
{
	int     sucesso = 0;
	int     desocupado, seta;
	int     i, i2;

	for (i = 0; i < 8; i++) {
		sucesso = 0;
		desocupado = 0;
		seta = 0;
		for (i2 = 0; i2 < 3; i2++) {
			if (tabuleiro[sequencias[i][i2]] == LEGENDA_USUARIO)
				sucesso++;
			if (tabuleiro[sequencias[i][i2]] == LEGENDA_VAZIO) {
				desocupado = sequencias[i][i2];
				seta = 1;
			}
		}
		if ((sucesso == 2) && seta)
			return (desocupado);
	}
	return (-1);
}

// ------------------------------------------------------------------------------
//    funcao: tenta_fechar
// descricao: verifica se nao existe alguma sequencia do jogador X por fechar
//        in: nada
//       out: -1 - nao ha sequencia
//            > -1 - quadrante que fecha
int
tenta_fechar (void)
{
	int     sucesso;
	int     desocupado, seta;
	int     i, i2;

	for (i = 0; i < 8; i++) {
		sucesso = 0;
		desocupado = 0;
		seta = 0;
		for (i2 = 0; i2 < 3; i2++) {
			if (tabuleiro[sequencias[i][i2]] == LEGENDA_X)
				sucesso++;
			if (tabuleiro[sequencias[i][i2]] == LEGENDA_VAZIO) {
				desocupado = sequencias[i][i2];
				seta = 1;
			}
		}
		if ((sucesso == 2) && seta)
			return (desocupado);
	}
	return (-1);
}

// ------------------------------------------------------------------------------
//    funcao: chuta_jogada
// descricao: como ultima opcao de jogada para X, chuta um quadrante qualquer
//        in: nada
//       out: quadrante livre
int
chuta_jogada (void)
{
	srand ((int) time (NULL));
	return (livre[(int) ((float) random () / (float) RAND_MAX * livre_max)]);
}

// ------------------------------------------------------------------------------
//    funcao: analisa_jogo
// descricao: analisa a melhor jogada, verificando as chances de cada jogador
//        in: nada
//       out: -1 - nao ha sequencia
//            > -1 - quadrante para tentar criar sequencia
int
analisa_jogo (void)
{
        int     jogadas_usuario;
        int     jogadas_X;
        int     i, i2, maior_chance_X, maior_chance_usuario;
        int     status_jogo[8][2]; // numero de jogadas em cada sequencia
        int     chance[9][2];
        int     possibilidades_de_jogadas[9];
        int     limite_possibilidades;

        // contabiliza o numero de jogadas do usuario e do X
        for (i = 0; i < 8; i++) {
                jogadas_usuario = 0;
                jogadas_X = 0;
                for (i2 = 0; i2 < 3; i2++) {
                        if (tabuleiro[sequencias[i][i2]] == LEGENDA_USUARIO)
                                jogadas_usuario++;
                        if (tabuleiro[sequencias[i][i2]] == LEGENDA_X)
                                jogadas_X++;
                }
                status_jogo[i][0] = jogadas_X;
                status_jogo[i][1] = jogadas_usuario;
        }

        // zera a matriz... *preguiça*
        for (i = 0; i < 9; i++) {
                chance[i][0] = 0;
                chance[i][1] = 0;
        }

        // estima a chance de jogo em cada _quadrante_ para cada jogador
        for (i = 0; i < 8; i++) 
                for (i2 = 0; i2 < 3; i2++) {    
                        if (tabuleiro[sequencias[i][i2]] == LEGENDA_VAZIO && status_jogo[i][0] > 0 && status_jogo[i][1] == 0)
                                chance[sequencias[i][i2]][0]++;
                        if (tabuleiro[sequencias[i][i2]] == LEGENDA_VAZIO && status_jogo[i][1] > 0 && status_jogo[i][0] == 0)
                                chance[sequencias[i][i2]][1]++;
                }

        // rotina p'ra verificar se existe alguma chance
        // se nao houver, cai fora...
        for (i = 0; i < 8 && (((chance[i][0] == chance[i][1]) == chance[i][0]) == 0); i++);
        if (i == 8)
                return(-1);

        // seleciona _quadrante_ com maior probabilidade d jogo
        // aqui a porca torce o rabo... :DDDD
        limite_possibilidades = 0;
        maior_chance_X = -1;
        maior_chance_usuario = -1;
        for (i = 0; i < 9; i++) {
                if ( game_mode == JOGO_DEFENSIVO ? chance[i][1] > maior_chance_usuario : chance[i][0] > maior_chance_X) {
                        limite_possibilidades = 0;
                        possibilidades_de_jogadas[limite_possibilidades] = i;
                        maior_chance_X = chance[i][0];
                        maior_chance_usuario = chance[i][1];
                } else {
                        if ( game_mode == JOGO_DEFENSIVO ? chance[i][1] == maior_chance_usuario : chance[i][0] == maior_chance_X) {
                                if ( game_mode == JOGO_DEFENSIVO ? chance[i][0] > maior_chance_X : chance[i][1] > maior_chance_usuario) {
                                        limite_possibilidades = 0;
                                        possibilidades_de_jogadas[limite_possibilidades] = i;
                                        if (game_mode == JOGO_DEFENSIVO) 
					        maior_chance_X = chance[i][0];
					else
					        maior_chance_usuario = chance[i][1];
                                } else if ( game_mode == JOGO_DEFENSIVO ? chance[i][0] == maior_chance_X : chance[i][1] == maior_chance_usuario) {
                                        limite_possibilidades++;
                                        possibilidades_de_jogadas[limite_possibilidades] = i; 
                                }
                        }
                }
        }

        // seleciona a jogadas que estao em "limite_possibilidades"
        i = (float) random () / (float) RAND_MAX * (limite_possibilidades+1);

        return(possibilidades_de_jogadas[i]);
}

// ------------------------------------------------------------------------------
//    funcao: tenta_matar_jogada
// descricao: procura matar uma sequencia que o usuario planeja fazer
//        in: nada
//       out: -1 - nao ha sequencia
//            > -1 - quadrante para tentar criar sequen`cia
int
tenta_matar_jogada (void)
{
	int     sucesso;
	int     jogadasX;
	int     i, i2, maior_chance, opcao_escolhida, nr_opcoes;
	int     p = -1;
	int     provavel_jogada[8][2];
	int     chance[9] =
	{0, 0, 0, 0, 0, 0, 0, 0, 0};

	// procura por jogadas em aberto,
	// contabilizando o numero de quadrantes jah ocupados das sequencias
	for (i = 0; i < 8; i++) {
		sucesso = 0;
		jogadasX = 0;
		for (i2 = 0; i2 < 3; i2++) {
			if (tabuleiro[sequencias[i][i2]] == LEGENDA_USUARIO)
				sucesso++;
			if (tabuleiro[sequencias[i][i2]] == LEGENDA_X)
				jogadasX++;
		}
		if (jogadasX == 0) {
			p++;
			provavel_jogada[p][0] = i;
			provavel_jogada[p][1] = sucesso;
		}
	}

	// procura jogar nas sequencias q jah tem casa preenchida
	if (p >= 0) {
		for (i = 0; i <= p; i++)
			if (provavel_jogada[i][1] >= 0)
				for (i2 = 0; i2 < 3; i2++)
					if (tabuleiro[sequencias[provavel_jogada[i][0]][i2]] == LEGENDA_VAZIO)
						chance[sequencias[provavel_jogada[i][0]][i2]]++;
		maior_chance = chance[0];
		nr_opcoes = 1;
		for (i = 1; i <= 8; i++) {
			if (maior_chance < chance[i]) {
				nr_opcoes = 1;
				maior_chance = chance[i];
			} else {
				if (maior_chance == chance[i]) {
					nr_opcoes++;
				}
			}
		}

		opcao_escolhida = (float) random () / (float) RAND_MAX *(nr_opcoes);

		i2 = 0;
		for (i = 0; i <= 8; i++) {
			if ((maior_chance == chance[i]) && (maior_chance >= 1)) {
				if (i2 == opcao_escolhida)
					return (i);
				i2++;
			}
		}
	}
	// se nao tiver jeito, passa adiante... :)
	return (-1);
}


// ------------------------------------------------------------------------------
//    funcao: jogaX
// descricao: cerebro do jogador X, nesta rotina que ele escolhe onde jogara
//        in: nada
//       out: nada
void
jogaX (void)
{
	int     q;

	q = tenta_fechar ();
	if (q == -1)
	       	q = tapa_buraco ();
	if (q == -1)
	        q = analisa_jogo ();
	if (q == -1)
	        q = chuta_jogada();

	livre_desempilha (q);

	tabuleiro[q] = LEGENDA_X;

	desenhaX (q);

	game_over ();
}

// ------------------------------------------------------------------------------
//    funcao: jogaHumano
// descricao: rotina de verificacao da jogada do usuario
//        in: quadrante clicado pelo usuario
//       out: game over?
void
jogaHumano (int quadrante)
{
	if (adversario && isDeadmatch)
		desenhaAdversario (quadrante);
        else
		desenhaJogador (quadrante);
	adversario = !adversario;
	livre_desempilha (quadrante);
	game_over ();
}

// ------------------------------------------------------------------------------
//    funcao: usage
// descricao: help da aplicacao
//        in: nada
//       out: nada
void
usage (void)
{
	fprintf (stderr, "\nwmTicTacToe %s - The Ultimate TicTacToe for WindowMaker... :) \n", WMTICTACTOE_VERSION);
	fprintf (stderr, "Copyright © 1999 André Ribeiro Camargo\n\n");
	fprintf (stderr, "usage:\n");
	fprintf (stderr, "\t-display <display name>\n");
	fprintf (stderr, "\t-deadmatch\n");
	fprintf (stderr, "\t-h\tthis screen\n");
	fprintf (stderr, "\t-v\tprint the version number\n");
	fprintf (stderr, "\t-o\tofensive mode\n");
	fprintf (stderr, "\t-q\tquiet mode(for Debian's user)\n");
	fprintf (stderr, "\t\tdefault: defensive mode\n");
	fprintf (stderr, "\n");
}

// ------------------------------------------------------------------------------
//    funcao: printversion
// descricao: imprime a versao da aplicacao
//        in: nada
//       out: nada
void
printversion (void)
{
	if (!strcmp (ProgName, "wmtictactoe"))
		fprintf (stderr, "%s\n", WMTICTACTOE_VERSION);
}

// ------------------------------------------------------------------------------
//    funcao: readfile
// descricao: lê o arquivo de configuracao da aplicação
//        in: nada
//       out: nada
void 
readfile (void)
{
        FILE *rcfile;
	char rcfilen[256];
	char buf[256];
	int done;

	sprintf(rcfilen, "%s/.wmtictactoe", getenv("HOME"));

	if ((rcfile=fopen(rcfilen, "r")) != NULL){
	        do {
		        fgets(buf, 250, rcfile);
			if((done = feof(rcfile)) == 0){
			        buf[strlen(buf)-1]=0;
				if(strncmp(buf, "score_user_offensive ", strlen("score_user "))==0)
				        sscanf(buf, "score_user_offensive %i", &score_user_offensive);
				if(strncmp(buf, "score_X_offensive ", strlen("score_X "))==0)
				        sscanf(buf, "score_X_offensive %i", &score_X_offensive);
				if(strncmp(buf, "score_deuce_offensive ", strlen("score_deuce "))==0)
				        sscanf(buf, "score_deuce_offensive %i", &score_deuce_offensive);

				if(strncmp(buf, "score_user_defensive ", strlen("score_user "))==0)
				        sscanf(buf, "score_user_defensive %i", &score_user_defensive);
				if(strncmp(buf, "score_X_defensive ", strlen("score_X "))==0)
				        sscanf(buf, "score_X_defensive %i", &score_X_defensive);
				if(strncmp(buf, "score_deuce_defensive ", strlen("score_deuce "))==0)
				        sscanf(buf, "score_deuce_defensive %i", &score_deuce_defensive);
			}
		 } while(done == 0);
		 fclose(rcfile);
	}
}

// ------------------------------------------------------------------------------
//    funcao: writefile
// descricao: grava o arquivo de configuracao da aplicação
//        in: nada
//       out: nada
void 
writefile (void)
{
        FILE *rcfile;
	char rcfilen[256];

	sprintf(rcfilen, "%s/.wmtictactoe", getenv("HOME"));

	if ((rcfile=fopen(rcfilen, "w")) != NULL){
                fprintf(rcfile, "score_user_offensive %d\nscore_deuce_offensive %d\nscore_X_offensive %d\n", score_user_offensive, score_deuce_offensive, score_X_offensive);
                fprintf(rcfile, "score_user_defensive %d\nscore_deuce_defensive %d\nscore_X_defensive %d\n", score_user_defensive, score_deuce_defensive, score_X_defensive);
		fclose(rcfile);
	}
}

// ------------------------------------------------------------------------------
//    funcao: troca_game_mode
// descricao: troca o modo de jogo
//        in: nada
//       out: nada
void 
troca_game_mode (void)
{
        game_mode = !game_mode;

	if (!mute_mode)
	        fprintf (stderr, "\nPlaying on %s mode... %s", (game_mode == JOGO_DEFENSIVO) ? "DEFENSIVE" : "OFFENSIVE", (game_mode == JOGO_DEFENSIVO) ? ":(" : ":)");

        reseta_tabuleiro ();
}


