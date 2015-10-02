/*
 * file_parser.c
 *
 *  Created on: 16/giu/2015
 *      Author: luca
 *
 * Per ogni motore viene genarato un file con i punti da raggiungere ed il tempo
 * in cui raggiungerli. Sarà cura del modulo di conversione rpy-punti effettuare
 * tutti i controlli per quanto riguarda sia le velocità massime raggiunte,
 * il rispetto dei tempi minimi e che la somma di tutti i tempi per ogni motore
 * sia uguale rispetto a tutti gli altri.
 * La directory di default dove trovare i file sarà ./table
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "CANOpenShellMasterError.h"
#include "file_parser.h"
#include "CANOpenShell.h"

long row_read[127];
long row_total[127];
float compleate_percent;

float FileCompleteGet(int nodeId, int point_in_table)
{
  if(row_total[nodeId] != 0)
  {
//    printf("nodeid %d row %ld row total %ld point %d\n", nodeId, row_read[nodeId], row_total[nodeId], point_in_table);
    return (((row_read[nodeId] - point_in_table) * 100) / row_total[nodeId]);
  }
  else
    return 0;
}

long FileLineCount(int nodeId)
{
  FILE *file = NULL;
  char *line = NULL;
  size_t len = 0;

  char file_path[256];
  long line_count = 0;

  if(fake_flag == 0)
    sprintf(file_path, "%s%d.mot", FILE_DIR, nodeId);
  else
    sprintf(file_path, "%s%d.mot.fake", FILE_DIR, nodeId);

  file = fopen(file_path, "r");

  if(file == NULL)
  {
#ifdef CANOPENSHELL_VERBOSE
    if(verbose_flag)
      perror("file");
#endif
    return -1;
  }

  // Ogni informazione è delimitata da uno spazio
  while(getline(&line, &len, file) != -1)
    line_count++;

  free(line);
  fclose(file);

  return line_count;
}

void *QueueRefiller(void *args)
{
  struct table_data *data = args;
  int data_refilled = 0;
  int err;

  err = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  if(err != 0)
    printf("can't set thread as cancellable\n");

  err = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

  if(err != 0)
    printf("can't set thread as cancellable deferred\n");

  row_total[data->nodeId] = FileLineCount(data->nodeId);

  data->end_reached = 0;

  while(1)
  {
    pthread_testcancel();

    pthread_mutex_lock(&data->table_mutex);

    if(data->count < POSITION_DATA_NUM_MAX)
    {
      data_refilled = QueuePut(data, POSITION_DATA_NUM_MAX - data->count);

      if(data_refilled == -1)
      {
#ifdef CANOPENSHELL_VERBOSE
        if(verbose_flag)
        {
          printf("ERR[%d on node %x]: Errore nel file.\n", InternalError,
              data->nodeId);
        }
#endif

        break;
      }
      /*else if(data_refilled > 0)
      {
        printf("Refilled node %d: %d\n", data->nodeId, data_refilled);
        fflush(stdout);
      }*/

      pthread_mutex_unlock(&data->table_mutex);
    }
    else
      pthread_mutex_unlock(&data->table_mutex);

    usleep(10000);
  }

  return NULL;
}

void QueueInit(int nodeid, struct table_data *data)
{
  void *res;
  if(data->table_refiller != 0)
  {
    if(pthread_cancel(data->table_refiller) == 0)
    {
      pthread_join(data->table_refiller, &res);

      if(res == PTHREAD_CANCELED)
        data->table_refiller = 0;
    }
  }

  data->nodeId = nodeid;
  data->write_pointer = 0;
  data->read_pointer = 0;
  data->count = 0;
  data->cursor_position = 0;
  data->end_reached = 0;

  row_read[nodeid] = 0;
}

void QueueFill(struct table_data *data)
{
  row_read[data->nodeId] = 0;
  data->write_pointer = 0;
  data->read_pointer = 0;
  data->count = 0;
  data->cursor_position = 0;
  data->end_reached = 0;

  if(data->table_refiller == 0)
  {
    int err;
    err = pthread_create(&data->table_refiller, NULL, &QueueRefiller, data);

    if(err != 0)
      printf("can't create thread:[%s]", strerror(err));
  }
}

/**
 * Legge dalla coda e restituisce i valori nella struttura passata.
 *
 * Questa funzione si occupa anche di seguire il buffer circolare a seconda dell'offset
 * impostato. L'aggiornamento dei puntatori viene lasciato alla funzione QueueUpdate: in
 * questo modo è possibile leggere nuovamente i dati in caso di errore.
 */
int QueueGet(struct table_data *data_in, struct table_data_read *data_out,
    int offset)
{
  int read_pointer;

  // Controllo che l'offset non sia più grande dei dati che ho scritto nel buffer
  if(offset > data_in->count)
  {
    #ifdef CANOPENSHELL_VERBOSE

    if(verbose_flag)
    {
      printf("Superato il limite del file\n");
    }
#endif
    return -2;
  }

  if((data_in->read_pointer + offset) < POSITION_DATA_NUM_MAX)
    read_pointer = data_in->read_pointer + offset;
  else
    read_pointer = (data_in->read_pointer + offset) % POSITION_DATA_NUM_MAX;

  if(data_in->type != 'S')
  {
#ifdef CANOPENSHELL_VERBOSE
    if(verbose_flag)
    {
      printf("Errore nella riga di movimento: %d %c\n", data_in->nodeId, data_in->type);
    }
#endif

    return -1;
  }

  data_out->position = data_in->position[read_pointer];
  data_out->time_ms = data_in->time_ms[read_pointer];

  return 0;
}

/**
 * Legge l'ultimo dato  nella coda.
 *
 * @attention: assicurarsi che la coda sia stata scritta precedentemente, altrimenti
 * verrà restituito un valore casuale.
 */
int QueueLast(struct table_data *data_in, struct table_data_read *data_out)
{
  int read_pointer;

  if((data_in->read_pointer) > 0)
    read_pointer = data_in->read_pointer - 1;
  else
    read_pointer = POSITION_DATA_NUM_MAX - 1;

  data_out->position = data_in->position[read_pointer];
  data_out->time_ms = data_in->time_ms[read_pointer];

  return 0;
}

/**
 * Aggiorna le variabili del buffer circolare.
 *
 * I valori del buffer vengono presi direttamente dalla struttura ed questa funzione
 * dovrebbe essere richiamata solo quando si è certi che i dati siano arrivati a
 * destinazione.
 *
 */
void QueueUpdate(struct table_data *data, int point_number)
{
  if(point_number <= data->count)
  {
    data->read_pointer += point_number;

    if(data->read_pointer >= POSITION_DATA_NUM_MAX)
      data->read_pointer = data->read_pointer % POSITION_DATA_NUM_MAX;

    data->count -= point_number;
  }
  else
  {
    if(data->end_reached == 0)
      printf(
          "ERR[%d on node %x]: punti incongruenti con il buffer (QueueUpdate).\n",
          InternalError, data->nodeId);
  }

}

/**
 * Aggiunge alla coda i valori letti dal file di simulazione.
 *
 * @return >= 0: letta una nuova riga; -1: errore file -2: buffer pieno
 *
 * @remark: nella funzione è integrato un controllo sulla sintassi della riga letta.
 * Nel caso in cui quest'ultima non rispetti le regole prefisse, viene automaticamente
 * scartata e si passa alla prossima, fino al raggiungimento di numero di righe valide
 * passate come parametro oppure alla fine del file. Comunque, quando viene riscontrato
 * un errore, il parametro "type" viene sovrascritto con "E". Questo serve soprattutto
 * per le righe di homing, che sono sempre singole e che altrimenti non avrebbero nessun
 * parametro di errore da passare all'utilizzatore (le righe tabella hanno come indicatore
 * il numero di dati letti "count").
 * Ci sono i seguenti tipi di riga:
 *    "H": contiene i parametri per l'homing, in particolare l'offset dal limite di
 *         giunto e le velocità di andata e ritorno. La riga di homing deve essere
 *         la prima e l'unica del file, in questo questa operazione non può essere
 *         eseguita più volte con valori diversi.
 *
 *    "S": contiene i parametri da passare alla tabella dell'interpolatore.
 */
int QueuePut(struct table_data *data, int line_number)
{
  FILE *file = NULL;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  char *token;
  char *token_save;
  char line_copy[256];
  char file_path[256];
  char position[12];
  char time[12];
  char vel_forw[12];
  char vel_backw[12];
  char nodeid[3];
  int line_count = 0;

  if(data->end_reached == 1)
    return 0;

  if(data->count == POSITION_DATA_NUM_MAX)
    return -2;

  if(fake_flag == 0)
    sprintf(file_path, "%s%d.mot", FILE_DIR, data->nodeId);
  else
    sprintf(file_path, "%s%d.mot.fake", FILE_DIR, data->nodeId);



  file = fopen(file_path, "r");

  if(file == NULL)
  {
#ifdef CANOPENSHELL_VERBOSE
    if(verbose_flag)
      perror("file");
#endif
    return -1;
  }

  if(fseek(file, data->cursor_position, SEEK_SET) != 0)
  {
    fclose(file);
    return -1;
  }

  for(line_count = 0; line_count < line_number; line_count++)
  {
    // Ogni informazione è delimitata da uno spazio
    if((read = getline(&line, &len, file)) != -1)
    {
      if(strcmp(line, "\n") == 0)
        continue;

      // controllo se la riga entra nel buffer
      if(len < sizeof(line_copy))
        strcpy(line_copy, line);
      else
        strncpy(line_copy, line, sizeof(line_copy));

      // Verifico la sintassi
      if(strncmp(line_copy, "CT1 M", strlen("CT1 M")))
      {
#ifdef CANOPENSHELL_VERBOSE
        if(verbose_flag)
        {
          printf("WARN[%d on node %x]: Riga %ld non valida (sintassi CT1 M)\n",
              InternalError, data->nodeId, row_read[data->nodeId] + line_count);

          printf("line: %s", line);
        }
#endif
        data->type = 'E';
        data->cursor_position += read;

        continue;
      }

      // controllo che l'indirizzo del motore combaci
      token = strtok_r(line_copy + 5, " ", &token_save);
      if(token != NULL)
      {
        strcpy(nodeid, token);
        if(atoi(nodeid) != data->nodeId)
        {
#ifdef CANOPENSHELL_VERBOSE
          if(verbose_flag)
          {
            printf("WARN[%d on node %x]: Riga %ld non valida (nodeid)\n",
                InternalError, data->nodeId,
                row_read[data->nodeId] + line_count);

            printf("line: %s", line);
          }
#endif

          data->type = 'E';
          data->cursor_position += read;
          continue;
        }
      }
      else
      {
#ifdef CANOPENSHELL_VERBOSE
        if(verbose_flag)
        {
          printf("WARN[%d on node %x]: Riga %ld non valida (sintassi nodeid)\n",
              InternalError, data->nodeId, row_read[data->nodeId] + line_count);

          printf("line: %s", line);
        }
#endif

        data->cursor_position += read;
        continue;
      }

      // determino il tipo di riga letta
      token = strtok_r(NULL, " ", &token_save);
      if(token != NULL)
      {
        data->type = *token;
        strcpy(position, token + 1);
      }
      else
      {
#ifdef CANOPENSHELL_VERBOSE
        if(verbose_flag)
        {
          printf("WARN[%d on node %x]: Riga %ld non valida (sintassi tipo)\n",
              InternalError, data->nodeId, row_read[data->nodeId] + line_count);

          printf("line: %s", line);
        }
#endif

        data->type = 'E';
        data->cursor_position += read;
        continue;
      }

      // a seconda del tipo di dato, leggo i parametri successivi
      switch(data->type)
      {
        case 'S':
          // copio il tempo
          token = strtok_r(NULL, " \n\r\a", &token_save);
          if(token != NULL)
          {
            if(*token == 'T')
              strcpy(time, token + 1);
            else
              goto fault;
          }
          else
            goto fault;

          long lposition = atol(position);
          long ltime = atol(time);

          memcpy(&data->position[data->write_pointer], &lposition,
              sizeof(&data->position[data->write_pointer]));

          memcpy(&data->time_ms[data->write_pointer], &ltime,
              sizeof(&data->time_ms[data->write_pointer]));

          break;

        case 'H':
          // se non è la prima riga letta, allora la devo ignoare
          if((row_read[data->nodeId] != 0) || line_count != 0)
            goto fault;

          // copio le velocità di andata e ritorno
          token = strtok_r(NULL, " ", &token_save);
          if(token != NULL)
          {
            if(strncmp(token, "VF", 2) == 0)
              strcpy(vel_forw, token + 2);
            else
              goto fault;
          }
          else
            goto fault;

          token = strtok_r(NULL, " \n\r\a", &token_save);
          if(token != NULL)
          {
            if(strncmp(token, "VB", 2) == 0)
              strcpy(vel_backw, token + 2);
            else
              goto fault;
          }
          else
            goto fault;

          data->offset = atol(position);
          data->forward_velocity = atol(vel_forw);
          data->backward_velocity = atol(vel_backw);

          //return 1;
          break;

        default:
          fault:

#ifdef CANOPENSHELL_VERBOSE
          if(verbose_flag)
          {
            printf("WARN[%d on node %x]: Riga %ld non valida\n",
                InternalError, data->nodeId,
                row_read[data->nodeId] + line_count);

            printf("line: %s", line);
          }
#endif

          data->type = 'E';
          data->cursor_position += read;
          continue;
          break;
      }

      // aggiorno il buffer circolare
      data->cursor_position += read;
      data->write_pointer++;

      if(data->write_pointer >= POSITION_DATA_NUM_MAX)
        data->write_pointer = 0;

      data->count++;

      if(data->count == POSITION_DATA_NUM_MAX)
        break;
    }
    else
    {
      data->end_reached = 1;

      break;
    }
  }

  row_read[data->nodeId] += line_count + 1;

  free(line);
  fclose(file);

  return line_count;
}
