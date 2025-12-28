from flask import Flask, render_template, request, jsonify
from pymongo import MongoClient
import re

app = Flask(__name__)

client = MongoClient('mongodb://mongodb:27017/')
db = client['history_search']
collection = db['documents']

def highlight_text(text, query):
    pattern = re.compile(f'({re.escape(query)})', re.IGNORECASE)
    return pattern.sub(r'<mark>\1</mark>', text)

def strip_html(html):
    clean = re.sub('<[^<]+?>', ' ', html)
    clean = re.sub(r'\s+', ' ', clean)
    return clean.strip()

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/search', methods=['GET'])
def search():
    query = request.args.get('q', '')
    page = int(request.args.get('page', 1))
    per_page = 10
    
    if not query:
        return jsonify({'results': [], 'total': 0, 'page': page})
    
    regex = {'$regex': query, '$options': 'i'}
    
    total = collection.count_documents({'html_content': regex})
    
    skip = (page - 1) * per_page
    
    cursor = collection.find(
        {'html_content': regex},
        {'url': 1, 'html_content': 1, 'source': 1, '_id': 0}
    ).skip(skip).limit(per_page)
    
    results = []
    for doc in cursor:
        text = strip_html(doc['html_content'])
        
        pos = text.lower().find(query.lower())
        if pos != -1:
            start = max(0, pos - 100)
            end = min(len(text), pos + 150)
            snippet = text[start:end]
            if start > 0:
                snippet = '...' + snippet
            if end < len(text):
                snippet = snippet + '...'
        else:
            snippet = text[:200] + '...'
        
        snippet = highlight_text(snippet, query)
        
        results.append({
            'url': doc['url'],
            'snippet': snippet,
            'source': doc.get('source', 'unknown')
        })
    
    return jsonify({
        'results': results,
        'total': total,
        'page': page,
        'pages': (total + per_page - 1) // per_page
    })

@app.route('/api/stats')
def stats():
    total_docs = collection.count_documents({})
    sources = collection.aggregate([
        {'$group': {'_id': '$source', 'count': {'$sum': 1}}}
    ])
    
    return jsonify({
        'total_documents': total_docs,
        'sources': list(sources)
    })

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
