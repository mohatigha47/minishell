/*
  Projet minishell - Licence 3 Info - PSI 2023

  Nom : TIGHA
  Prénom :  MOHAMMED
  Num. étudiant : 22313911
  Groupe de projet : Binome 2
  Date :  23/11/2023

  Interface du minishell.

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "cmd.h"

int main(int argc, char *argv[])
{
  char cmdline[MAX_LINE_SIZE]; // buffer des lignes de commandes
  char *cmdtoks[MAX_CMD_SIZE]; // "mots" de la ligne de commandes
  cmd_t cmds[MAX_CMD_SIZE];
  cmd_t *current;

  while (1)
  {
    // Effacer les contenus de cmdline, cmdtoks et cmds
    // printf("PROGRAM STARTED MINISHELL\n");
    memset(cmdline, 0, MAX_LINE_SIZE);
    for (int i = 0; i < MAX_CMD_SIZE; i++)
    {
      cmdtoks[i] = NULL;
    }
    // Initilization de structure commande cmd_t
    init_cmd(cmds);
    // Afficher un prompt
    printf("$ ");

    // Lire une ligne dans cmdline - Attention fgets enregistre le \n final
    if (fgets(cmdline, MAX_LINE_SIZE, stdin) == NULL)
      break;
    cmdline[strlen(cmdline) - 1] = '\0';

    // Traiter la ligne de commande
    //   - supprimer les espaces en début et en fin de ligne
    //   - ajouter d'éventuels espaces autour de ; ! || && & ...
    //   - supprimer les doublons d'espaces
    //   - traiter les variables d'environnement
    trim(cmdline);
    clean(cmdline);

    // Découper la ligne dans cmdtoks
    strcut(cmdline, ' ', cmdtoks, MAX_CMD_SIZE);
    // printf("Number of tokens: %zu\n", tokenCount);
    //  Traduire la ligne en structures cmd_t dans cmds
    //  Les commandes sont chaînées en fonction des séparateurs
    //    - next -> exécution inconditionnelle
    //    - next_success -> exécution si la commande précédente réussit
    //    - next_failure -> exécution si la commande précédente échoue
    parse_cmd(cmdtoks, cmds, MAX_CMD_SIZE);
    // Exécuter les commandes dans l'ordre en fonction des opérateurs
    // de flux
    for (current = cmds; current != NULL;)
    {
      // Lancer la commande
      exec_cmd(current);

      if (current->wait)
      {
        waitpid(current->pid, NULL, 0);
      }

      // Vérifier l'état du commande est qu'elle est réussite d'exécuter ou non pour décider la commande suivante
      if (current->status == 0 && current->next_success != NULL)
      {
        current = current->next_success; // Executer la commande "next_success" si la commande actuelle réussite
      }
      else if (current->status != 0 && current->next_failure != NULL)
      {
        current = current->next_failure; // Executer la commande "next_failure" si la commande actuelle échoue
      }
      else
      {
        current = current->next; // Execution inconditionnel du commande suivante
      }
      fflush(stdout);
    }
  }

  fprintf(stderr, "\nGood bye!\n");
  return 0;
}
