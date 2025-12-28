mod config;
mod crawler;
mod database;
mod document;
mod html_parser;

use log::info;
use std::sync::Arc;
use std::process::Command;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    env_logger::init();
    
    let cfg = config::Config::from_file("config.yaml")?;
    info!("Configuration loaded successfully");
    
    let db = database::Database::new(&cfg.database).await?;
    info!("Connected to MongoDB");
    
    let crawler = Arc::new(crawler::Crawler::new(cfg.clone(), db));
    
    info!("Starting crawler with {} workers", cfg.crawler.max_workers);
    crawler.clone().start().await?;
    
    info!("Crawling completed. Exporting to JSON using mongoexport...");
    
    // Используем системную команду mongoexport (быстрее!)
    let output = Command::new("mongoexport")
        .arg("--uri")
        .arg(&cfg.database.uri)
        .arg("--db")
        .arg(&cfg.database.database_name)
        .arg("--collection")
        .arg(&cfg.database.collection)
        .arg("--out")
        .arg("/app/data/documents.json")
        .arg("--jsonArray")
        .output();
    
    match output {
        Ok(result) => {
            if result.status.success() {
                let stdout = String::from_utf8_lossy(&result.stdout);
                info!("Export successful: {}", stdout);
            } else {
                let stderr = String::from_utf8_lossy(&result.stderr);
                info!("mongoexport not available, using fallback method...");
                
                // Fallback: используем старый метод
                let db_export = database::Database::new(&cfg.database).await?;
                let count = db_export.export_to_json("/app/data/documents.json").await?;
                info!("Exported {} documents using Rust method", count);
            }
        },
        Err(_) => {
            info!("mongoexport command failed, using Rust export...");
            let db_export = database::Database::new(&cfg.database).await?;
            let count = db_export.export_to_json("/app/data/documents.json").await?;
            info!("Exported {} documents", count);
        }
    }
    
    Ok(())
}