use crate::config::Config;
use crate::database::Database;
use crate::document::Document;
use crate::html_parser::HtmlParser;
use log::{info, warn, error};
use reqwest::Client;
use std::collections::HashSet;
use std::sync::Arc;
use std::sync::atomic::{AtomicUsize, Ordering};
use tokio::sync::RwLock;
use tokio::time::{sleep, Duration};
use futures::stream::{self, StreamExt};

pub struct Crawler {
    config: Config,
    database: Database,
    client: Client,
    visited: Arc<RwLock<HashSet<String>>>,
    document_count: Arc<AtomicUsize>,
}

#[derive(Clone)]
struct CrawlItem {
    url: String,
    depth: u32,
    source: String,
}

impl Crawler {
    pub fn new(config: Config, database: Database) -> Self {
        let client = Client::builder()
            .user_agent(&config.crawler.user_agent)
            .timeout(Duration::from_secs(config.crawler.timeout_secs))
            .build()
            .expect("Failed to build HTTP client");

        Self {
            config,
            database,
            client,
            visited: Arc::new(RwLock::new(HashSet::new())),
            document_count: Arc::new(AtomicUsize::new(0)),
        }
    }

    pub async fn start(self: Arc<Self>) -> Result<(), Box<dyn std::error::Error>> {
        let mut queue = Vec::new();

        for source in &self.config.sources {
            for start_url in &source.start_urls {
                queue.push(CrawlItem {
                    url: start_url.clone(),
                    depth: 0,
                    source: source.name.clone(),
                });
            }
        }

        info!("Starting: {} URLs, limit {}", queue.len(), self.config.crawler.max_documents);

        for depth in 0..=self.config.crawler.max_depth {
            let current_count = self.document_count.load(Ordering::Relaxed);
            if current_count >= self.config.crawler.max_documents {
                info!("Limit reached: {}", current_count);
                break;
            }

            let current_level: Vec<_> = queue.iter()
                .filter(|item| item.depth == depth)
                .cloned()
                .collect();

            if current_level.is_empty() {
                break;
            }

            info!("Depth {}: {} URLs ({}/{})", depth, current_level.len(), current_count, self.config.crawler.max_documents);

            let results: Vec<(Vec<String>, bool)> = stream::iter(current_level)
                .map(|item| {
                    let crawler = self.clone();
                    async move { crawler.crawl_page_with_limit(item).await }
                })
                .buffer_unordered(self.config.crawler.max_workers)
                .collect()
                .await;

            let current_count = self.document_count.load(Ordering::Relaxed);
            if current_count >= self.config.crawler.max_documents {
                info!("Limit: {}", current_count);
                break;
            }

            for (links, success) in results {
                if success {
                    for url in links.into_iter().take(30) {
                        if self.should_crawl(&url) {
                            queue.push(CrawlItem {
                                url,
                                depth: depth + 1,
                                source: "Wikipedia".to_string(),
                            });
                        }
                    }
                }
            }
        }

        let final_count = self.document_count.load(Ordering::Relaxed);
        info!("Completed: {}", final_count);
        Ok(())
    }

    async fn crawl_page_with_limit(&self, item: CrawlItem) -> (Vec<String>, bool) {
        let current_count = self.document_count.load(Ordering::Relaxed);
        if current_count >= self.config.crawler.max_documents {
            return (Vec::new(), false);
        }

        let normalized = Document::normalize_url(&item.url);

        {
            let mut visited = self.visited.write().await;
            if visited.contains(&normalized) {
                return (Vec::new(), false);
            }
            visited.insert(normalized.clone());
        }

        if self.config.crawler.delay_ms > 0 {
            sleep(Duration::from_millis(self.config.crawler.delay_ms)).await;
        }

        let response = match self.client.get(&item.url).send().await {
            Ok(resp) => resp,
            Err(e) => {
                warn!("Fetch failed {}: {}", item.url, e);
                return (Vec::new(), false);
            }
        };

        if !response.status().is_success() {
            return (Vec::new(), false);
        }

        let html = match response.text().await {
            Ok(text) => text,
            Err(e) => {
                warn!("Read failed {}: {}", item.url, e);
                return (Vec::new(), false);
            }
        };

        let document = Document::new(item.url.clone(), html.clone(), item.source.clone());
        
        if let Err(e) = self.database.save_document(&document).await {
            error!("Save failed {}: {}", item.url, e);
            return (Vec::new(), false);
        }

        let count = self.document_count.fetch_add(1, Ordering::Relaxed) + 1;
        
        if count % 500 == 0 {
            info!("Progress: {}/{}", count, self.config.crawler.max_documents);
        }

        let links = HtmlParser::extract_links(&html, &item.url);
        (links, true)
    }

    fn should_crawl(&self, url: &str) -> bool {
        url.starts_with("http") &&
        (url.contains("wikipedia.org/wiki") || url.contains("histrf.ru")) &&
        !url.contains("Special:") &&
        !url.contains("File:") &&
        !url.contains("Category:") &&
        !url.contains("Template:") &&
        !url.contains("Help:") &&
        !url.contains("Talk:")
    }
}