

public class Consumer implements Runnable
{

	TheBoundedBuffer 	buffer;
	
	int 				consume;
	
	double 				accumulate;
	
	int					item_count;
	
	Consumer(TheBoundedBuffer buffer)
	{
		
		this.buffer = buffer;	
		consume = 0;	
		accumulate = 0.0;	
		item_count 	= 1000 * 1000;
		
	}
	
	void consume(double nothing)
	{	
		// outside loop, logs the totals computes the totals and progress and count downs
	}
		
	public void run() 
	{   
		//	System.out.println("test consumer");	   	   
		try
		{	   
			while(consume < item_count)
			{			
				double digit = buffer.get();	
				
				consume(digit);	   
				
				accumulate+=digit; 		
				
				consume++;
		   
				if(consume > 0)
				{		 
					
					if(consume % (100 * 1000) == 0)
					{		
						
						System.out.println("Consumer: Consumer  " + consume + " items, Cumulative value of consumed items=" + accumulate);			
						
					}	
					
				}	
				
			}		
			
		}
		
		catch(InterruptedException e)
		{		
			
			System.out.println("Consumer Interrupted");   
			
		}	   
		
   }
	
}
