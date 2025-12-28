use serde::{Deserialize, Serialize};
use sha2::{Digest, Sha256};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Document {
    pub url: String,
    pub normalized_url: String,
    pub html_content: String,
    pub source: String,
    pub crawled_at: i64,
    pub content_hash: String,
}

impl Document {
    pub fn new(url: String, html: String, source: String) -> Self {
        let normalized = Self::normalize_url(&url);
        let hash = Self::compute_hash(&html);
        
        Self {
            url,
            normalized_url: normalized,
            html_content: html,
            source,
            crawled_at: chrono::Utc::now().timestamp(),
            content_hash: hash,
        }
    }
    
    pub fn normalize_url(url_str: &str) -> String {  // ← ДОБАВЛЕНО pub
        if let Ok(mut url) = url::Url::parse(url_str) {
            url.set_fragment(None);
            
            let pairs: Vec<_> = url.query_pairs()  // ← УДАЛЕНО mut
                .filter(|(k, _)| !k.starts_with("utm_") && k != "fbclid" && k != "gclid")
                .collect();
            
            if pairs.is_empty() {
                url.set_query(None);
            } else {
                let query: Vec<String> = pairs.iter()
                    .map(|(k, v)| format!("{}={}", k, v))
                    .collect();
                url.set_query(Some(&query.join("&")));
            }
            
            url.to_string().to_lowercase()
        } else {
            url_str.trim().to_lowercase()
        }
    }
    
    fn compute_hash(content: &str) -> String {
        let mut hasher = Sha256::new();
        hasher.update(content.as_bytes());
        format!("{:x}", hasher.finalize())
    }
}
