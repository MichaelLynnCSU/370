
import java.util.Random;

public class Producer implements Runnable
{
	
	
	int 				counter;	
	
	TheBoundedBuffer 	buffer;	
	
	int 				insert;
	
	double 				accumulate;
	
	
	Producer(TheBoundedBuffer buffer)
	{
		
		this.buffer = buffer;		
		
		counter 	= 1000 * 1000;	
		
		insert	= 0;		
		
		accumulate 	= 0.0;
		
	}

	
	double produce()
	{
		
		Random random = new Random();		
		
		Double bufferElement = random.nextDouble() * 100.0;	
		
		return bufferElement;
		
	}
	
	
   public void run() 
   {	   
	//	System.out.println("test producer");	   
		try
		{
	   			while(counter >= 0)
			{		
	   			double digit = produce();	
	   			
				buffer.insert(digit);	
				
				accumulate +=digit;		
				
				insert++;
		   
				if(insert > 0)
				{
				
					if(insert % (100 * 1000) == 0)
					{		
						System.out.println("Producer: Generated  " + insert + " items, Cumulative value of generated items=" + accumulate);				   
					}
				}
			
				counter--;
		   
			}	
	   }
	   catch(InterruptedException e)
	   {	   
		   System.out.println("Producer: Interrupted");	   
	   }	   
   }
	
}
