/*
  Projet minishell - Licence 3 Info - PSI 2023

  Nom : TIGHA
  Prénom :  MOHAMMED
  Num. étudiant : 22313911
  Groupe de projet :  Binome 2
  Date :  24/11/2023

  Gestion des processus (implémentation).

 */

#include "cmd.h"
#include "builtin.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int exec_cmd(cmd_t *p)
{
  if (is_builtin(p->path))
  {
    int result = builtin(p); // Executer la commande builtin
    if (result == -1)
    {
      fprintf(stderr, "Error: Command not recognized\n");
      return -1;
    }
    return 0;
  }

  pid_t child_pid;
  int child_status;

  // Créer un nouveau processus
  child_pid = fork();

  if (child_pid == -1)
  {
    perror("fork");
    return -1;
  }

  if (child_pid == 0)
  {
    // Partie fils

    // Traitement des fichiers
    if (p->stdin != STDIN_FILENO)
    {
      dup2(p->stdin, STDIN_FILENO);
      close(p->stdin);
    }
    if (p->stdout != STDOUT_FILENO)
    {
      dup2(p->stdout, STDOUT_FILENO);
      close(p->stdout);
    }
    if (p->stderr != STDERR_FILENO)
    {
      dup2(p->stderr, STDERR_FILENO);
      close(p->stderr);
    }

    // Fermeture des fichiers
    for (int i = 0; i < MAX_CMD_SIZE && p->fdclose[i] != 0; i++)
    {
      close(p->fdclose[i]);
    }

    // Execution de la commande
    // printf("Execution du commande: %s\n", p->argv[0]);
    execvp(p->argv[0], p->argv);

    perror("execvp");
    _exit(EXIT_FAILURE);
  }
  else
  {
    // Partie père

    p->pid = child_pid; // Affectation du PID

    // Check if there is a command to write to a pipe
    if (p->stdout != STDOUT_FILENO && !p->wait)
    {
      close(p->stdout); // Close the write end of the pipe for background commands
    }

    // Attender si la commande va executer en arriere plan
    if (p->wait)
    {
      if (waitpid(child_pid, &child_status, 0) == -1)
      {
        perror("waitpid");
        return -1;
      }
      p->status = WEXITSTATUS(child_status);
    }
    else
    {
      printf("Background command: %s (PID %d)\n", p->path, p->pid);
    }
  }

  return 0;
}

int init_cmd(cmd_t *p)
{
  // Initialisation des valeurs par défaut dans cmds
  for (int i = 0; i < MAX_CMD_SIZE; i++)
  {
    p[i].stdin = STDIN_FILENO;
    p[i].stdout = STDOUT_FILENO;
    p[i].stderr = STDERR_FILENO;
    p[i].wait = 1;
    p[i].path = NULL;
    for (int j = 0; j < MAX_CMD_SIZE; j++)
    {
      p[i].argv[j] = NULL;
      p[i].fdclose[j] = 0;
    }
    p[i].next = NULL;
    p[i].next_success = NULL;
    p[i].next_failure = NULL;
  }
}

int parse_cmd(char *tokens[], cmd_t *cmds, size_t max)
{
  size_t cmdIndex = 0;
  size_t tokenIndex = 0;

  while (tokens[tokenIndex] != NULL)
  {
    // Vérification du symbole "&"
    if (strcmp(tokens[tokenIndex], "&") == 0)
    {
      tokenIndex++;
      continue;
    }

    // Initialisation du nouveau commande
    cmd_t *currentCmd = &cmds[cmdIndex];
    currentCmd->path = tokens[tokenIndex];
    currentCmd->wait = 1; // Valeur par défaut pour attendre le fils

    // Initialisation des arguments et les indicateurs de fermeture des fichiers
    size_t argIndex = 0;
    for (size_t i = 0; i < MAX_CMD_SIZE; i++)
    {
      currentCmd->argv[i] = NULL;
      currentCmd->fdclose[i] = 0;
    }

    // DÉtection des arguments (si le prochain symbole n'est pas un symbole spéciale pipe,semicolon..etc)
    while (tokens[tokenIndex] != NULL &&
           strcmp(tokens[tokenIndex], "|") != 0 &&
           strcmp(tokens[tokenIndex], ";") != 0 &&
           strcmp(tokens[tokenIndex], "&&") != 0 &&
           strcmp(tokens[tokenIndex], "||") != 0)
    {
      // vérification des rédirection pour les entrées et sorties
      if (strcmp(tokens[tokenIndex], "<") == 0)
      {
        // redirection entrée
        tokenIndex++;
        if (tokens[tokenIndex] != NULL)
        {
          currentCmd->stdin = open(tokens[tokenIndex], O_RDONLY);
          currentCmd->fdclose[argIndex] = 1;
        }
      }
      else if (strcmp(tokens[tokenIndex], ">") == 0)
      {
        // redirection sortie
        tokenIndex++;
        if (tokens[tokenIndex] != NULL)
        {
          currentCmd->stdout = open(tokens[tokenIndex], O_WRONLY | O_CREAT | O_TRUNC, 0644);
          currentCmd->fdclose[argIndex] = 1;
        }
      }
      else if (strcmp(tokens[tokenIndex], ">>") == 0)
      {
        // redirection sortie (ajouter)
        tokenIndex++;
        if (tokens[tokenIndex] != NULL)
        {
          currentCmd->stdout = open(tokens[tokenIndex], O_WRONLY | O_CREAT | O_APPEND, 0644);
          currentCmd->fdclose[argIndex] = 1;
        }
      }
      else if (strcmp(tokens[tokenIndex], "2>") == 0)
      {
        //  redirection sortie d'erreur
        tokenIndex++;
        if (tokens[tokenIndex] != NULL)
        {
          currentCmd->stderr = open(tokens[tokenIndex], O_WRONLY | O_CREAT | O_TRUNC, 0644);
          currentCmd->fdclose[argIndex] = 1;
        }
      }
      else if (strcmp(tokens[tokenIndex], ">&2") == 0)
      {
        // redirection stdout vers stderr
        currentCmd->stdout = STDERR_FILENO;
        currentCmd->fdclose[argIndex] = 1;
      }
      else if (strcmp(tokens[tokenIndex], "2>&1") == 0)
      {
        // redirection stderr vers stdout
        currentCmd->stderr = STDOUT_FILENO;
        currentCmd->fdclose[argIndex] = 1;
      }
      else if (strcmp(tokens[tokenIndex], "2>>") == 0)
      {
        // redirection sortie (ajouter)
        tokenIndex++;
        if (tokens[tokenIndex] != NULL)
        {
          currentCmd->stderr = open(tokens[tokenIndex], O_WRONLY | O_CREAT | O_APPEND, 0644);
          currentCmd->fdclose[argIndex] = 1;
        }
      }
      else if (strcmp(tokens[tokenIndex], "&") == 0)
      {
        // execution en arrière plan
        currentCmd->wait = 0;
      }
      else
      {
        // argument
        currentCmd->argv[argIndex++] = tokens[tokenIndex];
      }

      tokenIndex++;
    }

    // passer vers la commande suivante
    if (tokens[tokenIndex] != NULL)
    {
      if (strcmp(tokens[tokenIndex], ";") == 0)
      {
        // Si ";" est detecté on va exécuter inconditionnellemnt la commande suivante
        if (tokens[tokenIndex + 1] != NULL)
        {
          currentCmd->next = &cmds[cmdIndex + 1];
        }
        tokenIndex++;
      }
      else if (strcmp(tokens[tokenIndex], "|") == 0)
      {
        int tube[2];
        if (pipe(tube) == -1)
        {
          perror("pipe");
          exit(EXIT_FAILURE);
        }

        // Redirect the stdout of the current command to the write end of the pipe
        currentCmd->stdout = tube[1];
        // Redirect the stdin of the next command to the read end of the pipe
        cmds[cmdIndex + 1].stdin = tube[0];

        tokenIndex++; // Move to the next token after the pipe
      }
      else if (strcmp(tokens[tokenIndex], "&&") == 0)
      {
        // mettre la commande suivante dans  "next_success" si la commande actuelle réussite
        if (tokens[tokenIndex + 1] != NULL)
        {
          currentCmd->next_success = &cmds[cmdIndex + 1];
        }
        tokenIndex++;
      }
      else if (strcmp(tokens[tokenIndex], "||") == 0)
      {
        // mettre la commande suivante dans  "next_failure" si la commande actuelle échoue
        if (tokens[tokenIndex + 1] != NULL)
        {
          currentCmd->next_failure = &cmds[cmdIndex + 1];
        }
        tokenIndex++;
      }

      // passer vers la commande suivante
      cmdIndex++;
    }
  }

  return 0;
}
