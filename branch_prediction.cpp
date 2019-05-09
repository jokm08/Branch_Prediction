/*************************************************************************************************/
/*												 												 */
/*					BRANCH PREDICTOR					 										 */
/*												 												 */
/* Author      : Jose Kurian Manooparambil(jkm452)						 						 */
/* Class       : Computing Systems Architecture							 						 */
/* Institution : New York University, Tandon School Of Engineering				 				 */
/*************************************************************************************************/

#include<iostream>
#include<fstream>
#include<bitset>
#include<boost/dynamic_bitset.hpp>
#include<cstdlib>
#include<cmath>
#include<string>
#include<sstream>
using namespace std;

/*Define class to read m and k values from file config_new.txt*/

class read_config
{
	public:

	int m,k;
	fstream datafile;
	
	//Constructor to read m and k values from file
	read_config()
	{
		int Data;
		datafile.open("config_new.txt",ios::in);
		datafile>>Data;
		m = Data;
		datafile>>Data;
		k = Data;
	}

	~ read_config()
	{
		datafile.close();	
	} 
};

/*Define class to read address locations and actual branch state from*/
/*the file trace.txt*/

class read_trace
{
	public:

	int address,state,lines;

	bitset<1> *branch_state;

	unsigned int * program_counter;
		
	fstream tracefile;
	//Constructor to access file trace.txt
	read_trace()
	{	
		string line,hex_value;
		int i = 0, j =0;
		
		lines = 0;
		tracefile.open("trace.txt",ios::in);

		while(getline(tracefile,line))
			lines++;

		branch_state = new bitset<1>[lines];
		program_counter = new unsigned int[lines];

		tracefile.clear();
		tracefile.seekg(0,tracefile.beg);
	
		while(getline(tracefile,line))
		{
			branch_state[i] = bitset<1>(line[9]);	

			hex_value.assign(line,0,8);
			stringstream s(hex_value);
			s>>std::hex>>program_counter[i++];
		}
	}

	~ read_trace()
	{
		tracefile.close();
		delete[] branch_state;
		delete[] program_counter;
	}

};

/*Define class to write the branch predictions to file trace.txt.out*/

class write_trace
{

	public:

	fstream OutputTrace;

	write_trace()
	{
		OutputTrace.open("trace.txt.out",ios::out|ios::trunc);		
	} 

	~write_trace()
	{
		OutputTrace.close();
	}

	void write2file(bitset<1> WrtVal)
	{
		OutputTrace<<WrtVal.to_ulong()<<"\n";
	}
};


/*Define function to make branch prediction depending upon the current value*/
/*in the saturating counter*/

bitset<1> saturating_counter(bitset<2> CurrentState)
{
	bitset<1> Prediction;
	if(CurrentState == bitset<2>(3) || CurrentState == bitset<2>(2))
	{
		//Current State is strongly taken or weakly taken
		Prediction = bitset<1>(1);		
	}
	else if(CurrentState == bitset<2>(1) || CurrentState == bitset<2> (0))
	{
		//Current State is strongly not taken or weakly not taken
		Prediction = bitset<1>(0);
	}
	else
	{
		//Do nothing
	}

	return Prediction;
}

int main()
{

	int m,k,i,j,p,q,row,col,counter;
	int MissCounter = 0;
	float MissRate = 0.0;
	unsigned int address,temp,GlobalBHR_Val;
	bitset<2> ** table;
	bitset<2> CurrentState,NewState;
	bitset<1> * branch_state,Prediction;
	read_config RdObj;
	read_trace TrcObj;
	write_trace WrtObj;

	m = RdObj.m;
	k = RdObj.k;

	cout<<"m :"<<m<<"k :"<<k;

	boost::dynamic_bitset<> GlobalBHR(k,(pow(2,k)-1));

	counter = TrcObj.lines;

	row = pow(2,m); // Number of rows of table
	col = pow(2,k); // Number of cols of table

	table = new bitset<2>*[row];

	branch_state = TrcObj.branch_state;


	/*Dynamically create a 2-D array of dimensions row x col*/
	for(i = 0;i<row;i++)
	{
		table[i] = new bitset<2>[col];
	}
	
	/*Initialize saturatin table with value strongly taken*/
	for(i=0;i<row;i++)
	{
		for(j=0;j<col;j++)
		{
			table[i][j] = bitset<2> (3);
		}
	}

	
	for(i=0;i<counter;i++)
	{
		/*Extracting the row of the table from last m bits of Program Counter*/
		address = TrcObj.program_counter[i];
		temp = 0;
		
		for(p=0;p<m;p++)
		{
			temp = temp | (address)&(1<<p); 
		}
	
		/*Extracting the column from the value of Global Branch History Register*/
		GlobalBHR_Val = 0;
		for(p=0;p<GlobalBHR.size();p++)
		{
			GlobalBHR_Val = GlobalBHR_Val |(GlobalBHR[p]<<p);
		}
		
		/*Get the current state of the saturating counter and make a prediction*/
		CurrentState = table[temp][GlobalBHR_Val];

		Prediction = saturating_counter(CurrentState);

		WrtObj.write2file(Prediction);

		if(Prediction == branch_state[i])
		{

			/*If prediction and actual branch state match, store the following*/
			/*values in the saturating counter - */
			/*If cuurent state is strongly taken, saturation counter remains strongly taken*/
			/*If cuurent state is weakly taken, saturation counter changes to strongly taken*/
			/*If cuurent state is strongly not taken, saturation counter remains strongly not taken*/
			/*If cuurent state is weakly not taken, saturation counter changes to strongly not taken*/

			if(CurrentState == bitset<2>(0))
			{
				NewState = bitset<2>(0);
			}
			else if(CurrentState == bitset<2>(1))
			{
				NewState = bitset<2>(0);
			}
			else if(CurrentState == bitset<2>(2))
			{
				NewState = bitset<2>(3);
			}
			else if(CurrentState == bitset<2>(3))
			{
				NewState = bitset<2>(3);			
			}
			else
			{
				//Do nothing
			}		
		}
		else
		{
			/*If prediction and actual branch state do not match, store the following*/
			/*values in the saturating counter - */
			/*If cuurent state is strongly taken, saturation counter changes to weakly taken*/
			/*If cuurent state is weakly taken, saturation counter changes to strongly not taken*/
			/*If cuurent state is strongly not taken, saturation counter chnages to weakly not taken*/
			/*If cuurent state is weakly not taken, saturation counter changes to strongly taken*/

			if(CurrentState == bitset<2>(0))
			{
				NewState = bitset<2>(1);
			}
			else if(CurrentState == bitset<2>(1))
			{
				NewState = bitset<2>(3);
			}
			else if(CurrentState == bitset<2>(2))
			{
				NewState = bitset<2>(0);
			}
			else if(CurrentState == bitset<2>(3))
			{
				NewState = bitset<2>(2);			
			}
			else
			{
				//Do nothing
			}		

			/*Update the miss counter by 1 inorder to calculate the miss rate at the end*/
			MissCounter += 1;
		}
		
		table[temp][GlobalBHR_Val] = NewState;	

		for(q=0;q<GlobalBHR.size();q++)	
		{			
			if(q != (GlobalBHR.size()-1))
			{
				GlobalBHR[q] = GlobalBHR[q+1];
			}
			else
			{
				GlobalBHR[q] = branch_state[i].to_ulong();
			}
		}		
	}

	MissRate = float(MissCounter)/float(counter)*100;

	cout<<"\nNo of Misses : "<<std::dec<<MissCounter<<"\nMiss Rate : "<<MissRate<<"\n";
	
	for(i = 0;i<row;i++)
	{
  		delete[] table[i];
	}

	delete[] table;
	
	return 0;
}
