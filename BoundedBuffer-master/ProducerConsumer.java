

public class ProducerConsumer {

	
    public static void main(String[] args)
	{

        TheBoundedBuffer buffer;	
        
        Producer producer;	
        
        Consumer consumer;
        
        buffer = new TheBoundedBuffer();

        producer = new Producer(buffer);      

        consumer = new Consumer(buffer);       
        
        Thread threadProducer = new Thread(producer);    
        
        Thread threadConsumer = new Thread(consumer);    
        
        threadProducer.start();   
        
        threadConsumer.start();
		
        try
        {
			threadProducer.join();
			
			System.out.println("Producer: Finished generating 1,000,000 items");
			
			threadConsumer.join();
			
			System.out.println("Consumer: Finished consuming 1,000,000 items");       
		}
		catch(InterruptedException e)
		{
		
			System.out.println("Producer Interrupted");
	   
		}
	   
        System.out.println("Exiting!");
        
    }
	
}
