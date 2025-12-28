use crate::config::DatabaseConfig;
use crate::document::Document;
use mongodb::{Client, Collection, options::ClientOptions};
use mongodb::bson::doc;
use log::{info, warn};

pub struct Database {
    collection: Collection<Document>,
}

impl Database {
    pub async fn new(config: &DatabaseConfig) -> Result<Self, Box<dyn std::error::Error>> {
        let client_options = ClientOptions::parse(&config.uri).await?;
        let client = Client::with_options(client_options)?;
        
        let db = client.database(&config.database_name);
        let collection = db.collection::<Document>(&config.collection);
        
        Ok(Self { collection })
    }
    
    pub async fn save_document(&self, doc: &Document) -> Result<(), Box<dyn std::error::Error>> {
        let filter = doc! { "normalized_url": &doc.normalized_url };
        
        let existing = self.collection.find_one(filter.clone(), None).await?;
        
        if let Some(existing_doc) = existing {
            if existing_doc.content_hash != doc.content_hash {
                self.collection.replace_one(filter, doc, None).await?;
                info!("Updated: {}", doc.url);
            }
        } else {
            self.collection.insert_one(doc, None).await?;
            info!("Inserted: {}", doc.url);
        }
        
        Ok(())
    }
    
    pub async fn export_to_json(&self, output_path: &str) -> Result<usize, Box<dyn std::error::Error>> {
        use futures::stream::StreamExt;
        use std::fs::File;
        use std::io::{Write, BufWriter};
        use std::time::Instant;
        
        info!("Exporting to: {}", output_path);
        let start = Instant::now();
        
        let total = self.collection.count_documents(None, None).await?;
        info!("Total: {}", total);
        
        let file = File::create(output_path)?;
        let mut writer = BufWriter::with_capacity(1024 * 1024, file);
        
        let mut cursor = self.collection.find(None, None).await?;
        let mut count = 0;
        
        while let Some(result) = cursor.next().await {
            match result {
                Ok(doc) => {
                    let json = serde_json::to_string(&doc)?;
                    writeln!(writer, "{}", json)?;
                    count += 1;
                    
                    if count % 100 == 0 {
                        writer.flush()?;
                        info!("Exported {}/{}", count, total);
                    }
                },
                Err(e) => warn!("Error: {}", e),
            }
        }
        
        writer.flush()?;
        info!("Done: {} in {:.2}s", count, start.elapsed().as_secs_f64());
        
        Ok(count)
    }
}