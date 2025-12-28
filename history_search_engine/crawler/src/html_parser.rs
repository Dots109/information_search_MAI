use scraper::{Html, Selector};
use url::Url;

pub struct HtmlParser;

impl HtmlParser {
    pub fn extract_links(html: &str, base_url: &str) -> Vec<String> {
        let document = Html::parse_document(html);
        let selector = Selector::parse("a[href]").unwrap();
        let base = Url::parse(base_url).ok();
        
        let mut links = Vec::new();
        
        for element in document.select(&selector) {
            if let Some(href) = element.value().attr("href") {
                if let Some(ref base_url) = base {
                    if let Ok(absolute) = base_url.join(href) {
                        let url_str = absolute.to_string();
                        if Self::is_valid_url(&url_str) {
                            links.push(url_str);
                        }
                    }
                }
            }
        }
        
        links
    }
    
    fn is_valid_url(url: &str) -> bool {
        !url.starts_with("javascript:") &&
        !url.starts_with("mailto:") &&
        !url.starts_with("tel:") &&
        !url.starts_with("#")
    }
    
    pub fn is_history_related(url: &str, html: &str) -> bool {
        let lower_url = url.to_lowercase();
        let lower_html = html.to_lowercase();
        
        let history_keywords = vec![
            "истор", "войн", "царь", "импер", "револю", 
            "ancient", "medieval", "dynasty", "battle", "empire"
        ];
        
        for keyword in history_keywords {
            if lower_url.contains(keyword) || lower_html.contains(keyword) {
                return true;
            }
        }
        
        false
    }
}
