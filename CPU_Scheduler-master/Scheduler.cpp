#include <stdio.h>
#include <stdlib.h>
#include <memory.h>


struct Process {
	int Process_ID;
	int Arrival_Time;
	int Burst_Duration;
	int Priority;
	int TurnAroundTime;
	int WaitTime;
};
//**********************************************************
struct Gantt
{
	int Process_ID;
	int Arrival_Time;
	int End_Time;
};

void File_Content(struct  Process Processes[], int number);
void FCFS(struct Process Processes[], int number);
void SJFP(struct Process Processes[], int number);
void PRIORITY(struct Process Processes[], int number);
void RR(struct Process Processes[], int number, int quantum);
void Display_Output(struct  Process Processes[], int number, struct Gantt Rows[], int count, char* filename);



int main(int argc, char **argv)
{

	int number_of_pross = 0;

	if (argc != 2)
	{
		printf("Incorrect number of arguments.\n");
		return -1;
	}

	FILE* file1 = fopen(argv[1], "r");

	while (!feof(file1))
	{
		char  ch = fgetc(file1);
		if (ch == '\n')
		{
			number_of_pross++;
		}
	}
	struct Process Processes[number_of_pross];

	fseek(file1, 0, SEEK_SET);


	number_of_pross = 0;


	while (fscanf(file1, "%d,%d,%d,%d", &Processes[number_of_pross].Process_ID, &Processes[number_of_pross].Arrival_Time, &Processes[number_of_pross].Burst_Duration, &Processes[number_of_pross].Priority) != EOF)
	{
		number_of_pross++;
	}
	fclose(file1);

	File_Content(Processes, number_of_pross);
	FCFS(Processes, number_of_pross);
	SJFP(Processes, number_of_pross);
	PRIORITY(Processes, number_of_pross);

	return 0;
}
//--------------------------------------------------------
void File_Content(struct Process Processes[], int number)
{
	int i;
	printf("PROCESS ID\tARRIVAL TIME\tBURST DURATION\tPRIORITY\n");
	for (i = 0; i<number; i++)
	{

		printf("%d\t\t%d\t\t%d\t\t%d\n", Processes[i].Process_ID, Processes[i].Arrival_Time, Processes[i].Burst_Duration, Processes[i].Priority);

	}
}
//---------------------------------------------------------------
void Display_Output(struct  Process Processes[], int number, struct Gantt Rows[], int count, char *filename)
{
	FILE* file;
	int i;
	file = fopen(filename, "w");
	fprintf(file, "PROCESS ID\tARRIVAL TIME\tEND TIME\n");

	for (i = 0; i<count; i++)
	{
		fprintf(file, "%10d%10d%10d\n", Rows[i].Process_ID, Rows[i].Arrival_Time, Rows[i].End_Time);
	}

	float avgWait = 0;
	float avgTurnAround = 0;
	for (i = 0; i<number; i++)
	{
		avgWait += Processes[i].WaitTime;
		avgTurnAround += Processes[i].TurnAroundTime;
	}
	fprintf(file, "Average Wait Time=%f\n", avgWait / number);
	fprintf(file, "TurnAround Time=%f\n", avgTurnAround / number);
	// Assumption: the value of count recieved is greater than exact number of rows in the Gantt chart by 1. Dont forget to adjust the value if yours is not so.
	fprintf(file, "Throughput=%f\n", ((float)number / Rows[count - 1].End_Time));
	fclose(file);

}
//------------------------------------------------------------

/*
Note: Use function Display_Output to display the result. The Processes is the
updated Processes with their respective waittime, turnAround time.
Use the array of Gantt Struct to mimic the behaviour of Gantt Chart. Rows is the base name of the same array.
number is the number of processes which if not changed during processing, is taken care. count is the number of rows in the Gantt chart.

Number of Processes will not exceed 1000. Create and array of Gantt chart which can accomodate atleast 1000 process.
*/
// -------------FCFS----------------------------------------

void FCFS(struct Process Processes[], int number)
{
	//---------Start Processing

	Process * processes = new Process[number];

	for (int i = 0; i < number; ++i)
	{
		processes[i] = Processes[i];
	}


	Process t;

	for (int i = 0; i < number; i++)
	{

		for (int j = i + 1; j < number; j++)
		{

			Process & a = processes[i];

			Process & b = processes[j];

			if (a.Arrival_Time < b.Arrival_Time)
				continue;
			else if (a.Arrival_Time == b.Arrival_Time && a.Process_ID < b.Process_ID)
				continue;

			t = a;
			a = b;
			b = t;

		}

	}

	int * starttime = new int[number];
	int * bursttime = new int[number];

	for (int i = 0; i < number; ++i)
	{
		starttime[i] = processes[i].Arrival_Time;
		bursttime[i] = processes[i].Burst_Duration;
	}


	for (int i = 1; i < number; ++i)
	{
		processes[i].Arrival_Time = processes[i - 1].Burst_Duration;
		processes[i].Burst_Duration = processes[i - 1].Burst_Duration + processes[i].Burst_Duration;
	}


	for (int i = 0; i < number; ++i)
	{
		processes[i].WaitTime = processes[i].Arrival_Time - starttime[i];
	}

	for (int i = 0; i < number; ++i)
	{
		processes[i].TurnAroundTime = processes[i].WaitTime + bursttime[i];
	}


	Gantt * rows = new Gantt[number];

	int iLastTime = 0;
	for (int i = 0; i < number; ++i) {

		rows[i].Process_ID = processes[i].Process_ID;
		rows[i].Arrival_Time = processes[i].Arrival_Time;
		rows[i].End_Time = processes[i].Burst_Duration;
	}
	//---------End of Processing
	//Hint: Call function with arguments shown Display_Output(Processes,number,Rows,count,"FCFS");

	Display_Output(processes, number, rows, number, "FCFS");

	delete[] rows;
	delete[] starttime;
	delete[] bursttime;
	delete[] processes;

}




//---------------------SJFP---------------------------------------
void SJFP(struct Process Processes[], int number)
{

	//---------Start Processing

	Gantt * rows = new Gantt[1000];

	Process * processes = new Process[number];

	for (int i = 0; i < number; ++i)
	{
		processes[i] = Processes[i];
	}

	Process t;

	for (int i = 0; i < number; i++)
	{

		for (int j = i + 1; j < number; j++)
		{

			Process & a = processes[i];

			Process & b = processes[j];

			if (a.Arrival_Time < b.Arrival_Time)
				continue;
			else if (a.Arrival_Time == b.Arrival_Time && a.Process_ID < b.Process_ID)
				continue;

			t = a;
			a = b;
			b = t;

		}

	}

	int * starttime = new int[number];
	int * bursttime = new int[number];

	for (int i = 0; i < number; ++i)
	{
		starttime[i] = processes[i].Arrival_Time;
		bursttime[i] = processes[i].Burst_Duration;
	}
	int currenttime = 0;
	int maxTime = 0;
	int count = 0;
	int endBurst = -1;
	int qsize = 0;
	Process * queue = new Process[number];


	Process current;



	for (int i = 0; i < number; ++i)
	{

		maxTime = maxTime + processes[i].Burst_Duration;

	}

	//cout << "maxTime " << maxTime << endl;  //2652
	while (currenttime < maxTime)
	{
		// testing currenttime <= maxTime

		for (int i = 0; i < number; ++i)
		{

			if (processes[i].Arrival_Time == currenttime)
			{
				// brings processes that match the current time of queue
				queue[qsize++] = processes[i];

				for (int j = 0; j < qsize; j++)
				{

					for (int k = j + 1; k < qsize; k++)
					{

						Process & a = queue[j];

						Process & b = queue[k];

						if (a.Burst_Duration < b.Burst_Duration)
						{

							continue;

						}
						else if (a.Burst_Duration == b.Burst_Duration)
						{

							if (a.Arrival_Time < b.Arrival_Time)
							{

								continue;

							}
							else if (a.Arrival_Time == b.Arrival_Time)
							{

								if (a.Process_ID < b.Process_ID)
								{

									continue;

								}

							}

						}

						t = a;
						a = b;
						b = t;

					}

				}

			}

		}

		if (qsize > 0)
		{

			current = queue[0];

		}

		if (count == 0)
		{

			// first case
			rows[count].Process_ID = queue[0].Process_ID;

			rows[count].Arrival_Time = currenttime;

			//endBurst = queue[0].Burst_Duration; // end burst for the first case (26)
			//cout << "endBurst " << endBurst << endl;
			++count;

		}
		else if (qsize > 0 && queue[0].Burst_Duration > 0 && queue[0].Process_ID != rows[count - 1].Process_ID)
		{

			int current = rows[count - 1].Process_ID;

			rows[count - 1].End_Time = currenttime;

			//cout << " currenttime " << currenttime << endl;
			//cout << " Preempt check " << queue[0].Process_ID << " With BR " << queue[0].Burst_Duration << endl;

			int findCurrentID = 0;

			for (int j = 0; j < qsize; ++j)
			{

				if (current == queue[j].Process_ID)
				{

					findCurrentID = j;

					break;

				}
			}
			//queue[findCurrentID].Burst_Duration = queue[findCurrentID].Burst_Duration - 1;
			//cout << " found preemt " << queue[findCurrentID].Process_ID << " With BR " << queue[findCurrentID].Burst_Duration << endl << endl;
			//if (count >= rows.size())
			//{
			// break;
			//}

			rows[count].Process_ID = queue[0].Process_ID;

			rows[count].Arrival_Time = currenttime;

			//endBurst = queue[0].Burst_Duration;
			//queue[0].Burst_Duration--;
			//cout << " endBurst " << endBurst << endl;

			++count;

		}

		if (qsize > 0)
		{

			queue[0].Burst_Duration--;

			if (queue[0].Burst_Duration <= 0)
			{
				// Remove Rows[count-1].Process_ID from the queue
				if (currenttime >= maxTime - 1)
				{
					rows[count - 1].End_Time = currenttime + 1;
				}
				else
				{

					rows[count].Process_ID = queue[0].Process_ID;

					rows[count].Arrival_Time = currenttime;

					rows[count - 1].End_Time = currenttime;

				}

				//endBurst = queue[0].Burst_Duration; // end burst for the first case (26)
				qsize--;

				if (qsize > 0)
				{

					memmove(queue, &queue[1], qsize * sizeof(queue[0]));

				}

			}

			//++count;

		}

		if (qsize > 0)
		{

			endBurst = queue[0].Burst_Duration;

		}
		else
		{

			endBurst = -1;

		}

		++currenttime;

	}  // end while loop

	   //cout << "Burst Reached 4564 Remove: " << rows[count].Process_ID << endl; //cout << "endBurst " << endBurst << endl;
	for (int j = 0; j < number; ++j)
	{
		processes[j].WaitTime = -bursttime[j] - starttime[j];
		processes[j].TurnAroundTime = -starttime[j];
	}

	for (int i = 0; i < number; ++i)
	{
		int findCurrentID = 0;
		for (int j = count - 1; j >= 0; --j)
		{

			if (processes[i].Process_ID == rows[j].Process_ID)
			{

				findCurrentID = j;
				break;
			}
		}
		processes[i].WaitTime += rows[findCurrentID].End_Time;
		processes[i].TurnAroundTime += rows[findCurrentID].End_Time;
	}


	//---------End of Processing

	Display_Output(processes, number, rows, count, "SJF");

	delete[] queue;
	delete[] starttime;
	delete[] bursttime;
	delete[] processes;
	delete[] rows;

}


//------------------PRIORITY-------------------------------------
void PRIORITY(struct Process Processes[], int number)
{
	//---------Start Processing
	Gantt * rows = new Gantt[1000];

	Process * processes = new Process[number];

	for (int i = 0; i < number; ++i)
	{
		processes[i] = Processes[i];
	}

	Process t;

	for (int i = 0; i < number; i++)
	{

		for (int j = i + 1; j < number; j++)
		{

			Process & a = processes[i];

			Process & b = processes[j];

			if (a.Arrival_Time < b.Arrival_Time)
				continue;
			else if (a.Arrival_Time == b.Arrival_Time && a.Process_ID < b.Process_ID)
				continue;

			t = a;
			a = b;
			b = t;

		}

	}

	int * starttime = new int[number];
	int * bursttime = new int[number];

	for (int i = 0; i < number; ++i)
	{
		starttime[i] = processes[i].Arrival_Time;
		bursttime[i] = processes[i].Burst_Duration;
	}
	int currenttime = 0;
	int maxTime = 0;
	int count = 0;
	int endBurst = -1;
	int qsize = 0;
	Process * queue = new Process[number];


	Process current;



	for (int i = 0; i < number; ++i)
	{

		maxTime = maxTime + processes[i].Burst_Duration;

	}

	//cout << "maxTime " << maxTime << endl;  //2652
	while (currenttime < maxTime)
	{
		// testing currenttime <= maxTime

		for (int i = 0; i < number; ++i)
		{

			if (processes[i].Arrival_Time == currenttime)
			{
				// brings processes that match the current time of queue
				queue[qsize++] = processes[i];

				for (int j = 0; j < qsize; j++)
				{

					for (int k = j + 1; k < qsize; k++)
					{

						Process & a = queue[j];

						Process & b = queue[k];

						if (a.Priority < b.Priority)
						{

							continue;

						}
						else if (a.Priority == b.Priority)
						{

							if (a.Arrival_Time < b.Arrival_Time)
							{

								continue;

							}
							else if (a.Arrival_Time == b.Arrival_Time)
							{

								if (a.Process_ID < b.Process_ID)
								{

									continue;

								}

							}

						}

						t = a;
						a = b;
						b = t;

					}

				}

			}

		}

		if (qsize > 0)
		{

			current = queue[0];

		}

		if (count == 0)
		{

			// first case
			rows[count].Process_ID = queue[0].Process_ID;

			rows[count].Arrival_Time = currenttime;

			//endBurst = queue[0].Burst_Duration; // end burst for the first case (26)
			//cout << "endBurst " << endBurst << endl;
			++count;

		}
		else if (qsize > 0 && queue[0].Burst_Duration > 0 && queue[0].Process_ID != rows[count - 1].Process_ID)
		{

			int current = rows[count - 1].Process_ID;

			rows[count - 1].End_Time = currenttime;

			//cout << " currenttime " << currenttime << endl;
			//cout << " Preempt check " << queue[0].Process_ID << " With BR " << queue[0].Burst_Duration << endl;

			int findCurrentID = 0;

			for (int j = 0; j < qsize; ++j)
			{

				if (current == queue[j].Process_ID)
				{

					findCurrentID = j;

					break;

				}
			}
			//queue[findCurrentID].Burst_Duration = queue[findCurrentID].Burst_Duration - 1;
			//cout << " found preemt " << queue[findCurrentID].Process_ID << " With BR " << queue[findCurrentID].Burst_Duration << endl << endl;
			//if (count >= rows.size())
			//{
			// break;
			//}

			rows[count].Process_ID = queue[0].Process_ID;

			rows[count].Arrival_Time = currenttime;

			//endBurst = queue[0].Burst_Duration;
			//queue[0].Burst_Duration--;
			//cout << " endBurst " << endBurst << endl;

			++count;

		}

		if (qsize > 0)
		{

			queue[0].Burst_Duration--;

			if (queue[0].Burst_Duration <= 0)
			{
				// Remove Rows[count-1].Process_ID from the queue
				if (currenttime >= maxTime - 1)
				{
					rows[count - 1].End_Time = currenttime + 1;
				}
				else
				{

					rows[count].Process_ID = queue[0].Process_ID;

					rows[count].Arrival_Time = currenttime;

					rows[count - 1].End_Time = currenttime;

				}

				//endBurst = queue[0].Burst_Duration; // end burst for the first case (26)
				qsize--;

				if (qsize > 0)
				{

					memmove(queue, &queue[1], qsize * sizeof(queue[0]));

				}

			}

			//++count;

		}

		if (qsize > 0)
		{

			endBurst = queue[0].Burst_Duration;

		}
		else
		{

			endBurst = -1;

		}

		++currenttime;

	}  

	for (int j = 0; j < number; ++j)
	{
		processes[j].WaitTime = -bursttime[j] - starttime[j];
		processes[j].TurnAroundTime = -starttime[j];
	}

	for (int i = 0; i < number; ++i)
	{
		int findCurrentID = 0;
		for (int j = count - 1; j >= 0; --j)
		{

			if (processes[i].Process_ID == rows[j].Process_ID)
			{

				findCurrentID = j;
				break;
			}
		}
		processes[i].WaitTime += rows[findCurrentID].End_Time;
		processes[i].TurnAroundTime += rows[findCurrentID].End_Time;
	}


	//---------End of Processing

	Display_Output(processes, number, rows, count, "priority");

	delete[] queue;
	delete[] starttime;
	delete[] bursttime;
	delete[] processes;
	delete[] rows;

}