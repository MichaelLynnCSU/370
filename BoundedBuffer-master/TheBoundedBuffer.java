

public class TheBoundedBuffer
{

	double 		buffer[];
	
	int 		begin;	
	
	int 		insert;	
	
	int 		count;
	
	TheBoundedBuffer()
	{
		
		// set limit
		buffer 	= new double[1000];	
		
		begin 	= 0;	
		
		insert 	= 0;	
		
		count 	= 0;
		
	}
	
	// prevent block by using wait and notify
	synchronized void insert(double digit) throws InterruptedException
	{
		// notify and wait should only be called from inside a synchronized member function
		// otherwise while one is locking other may lock to and the former lock too
		// causing dead lock, race conditions
		while(count >= 1000)
		{
			
			// woken up by notify
			wait();
			
		}
		
		buffer[insert] = digit;		
		
		insert++;
		
		if(insert >= 1000)
		{
			
			insert = 0;	
			
		}
		
		count++;
		
		// notify wait() to wake up
		notify();
		
	}

	// prevent block by using wait and notify
	synchronized double get() throws InterruptedException
	{
		// notify and wait should only be called from inside a synchronized member function
		// otherwise while one is locking other may lock to and the former lock too
		// causing dead lock, race conditions
		while(count <= 0)
		{
			
			// woken up by notify
			wait();
			
		}
		
		double digits = buffer[begin];	
		
		begin++;
		
		if(begin >= 1000)
		{		
			
			begin = 0;
			
		}
		
		count--;
		
		// notify wait() to wake up
		notify();
		
		return digits;
		
	}

}
