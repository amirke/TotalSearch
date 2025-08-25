// Mobile menu toggle
document.addEventListener('DOMContentLoaded', function() {
    const hamburger = document.querySelector('.hamburger');
    const navMenu = document.querySelector('.nav-menu');

    if (hamburger && navMenu) {
        hamburger.addEventListener('click', function() {
            hamburger.classList.toggle('active');
            navMenu.classList.toggle('active');
        });
    }
});

// Smooth scrolling for navigation links
document.addEventListener('DOMContentLoaded', function() {
    const navLinks = document.querySelectorAll('a[href^="#"]');
    
    navLinks.forEach(link => {
        link.addEventListener('click', function(e) {
            e.preventDefault();
            const targetId = this.getAttribute('href');
            const targetSection = document.querySelector(targetId);
            
            if (targetSection) {
                targetSection.scrollIntoView({
                    behavior: 'smooth',
                    block: 'start'
                });
            }
        });
    });
});

// Navbar Background on Scroll
window.addEventListener('scroll', function() {
    const navbar = document.querySelector('.navbar');
    if (window.scrollY > 50) {
        navbar.style.background = 'rgba(255, 255, 255, 0.98)';
        navbar.style.boxShadow = '0 2px 20px rgba(0, 0, 0, 0.1)';
    } else {
        navbar.style.background = 'rgba(255, 255, 255, 0.95)';
        navbar.style.boxShadow = 'none';
    }
});

// Intersection Observer for Animations
const observerOptions = {
    threshold: 0.1,
    rootMargin: '0px 0px -50px 0px'
};

const observer = new IntersectionObserver(function(entries) {
    entries.forEach(entry => {
        if (entry.isIntersecting) {
            entry.target.style.opacity = '1';
            entry.target.style.transform = 'translateY(0)';
        }
    });
}, observerOptions);

// Observe elements for animation
document.addEventListener('DOMContentLoaded', function() {
    const animatedElements = document.querySelectorAll('.feature-card, .pricing-card, .download-card');
    animatedElements.forEach(el => {
        el.style.opacity = '0';
        el.style.transform = 'translateY(30px)';
        el.style.transition = 'opacity 0.6s ease, transform 0.6s ease';
        observer.observe(el);
    });
});

// Pricing Card Hover Effects
document.querySelectorAll('.pricing-card').forEach(card => {
    card.addEventListener('mouseenter', function() {
        this.style.transform = 'translateY(-10px) scale(1.02)';
    });
    
    card.addEventListener('mouseleave', function() {
        this.style.transform = 'translateY(0) scale(1)';
    });
});

// Download Button Click Handlers
document.querySelectorAll('.btn-download').forEach(button => {
    button.addEventListener('click', function(e) {
        e.preventDefault();
        
        // Show download modal or redirect to actual download
        const platform = this.textContent.includes('Windows') ? 'Windows' : 
                        this.textContent.includes('macOS') ? 'macOS' : 'Linux';
        
        // For now, show an alert - replace with actual download logic
        alert(`Download for ${platform} will be available soon!`);
        
        // Track download attempt (replace with actual analytics)
        console.log(`Download attempted for ${platform}`);
    });
});

// Feature Tag Animation
document.querySelectorAll('.feature-tag').forEach(tag => {
    tag.addEventListener('mouseenter', function() {
        this.style.transform = 'scale(1.05)';
        this.style.background = 'rgba(255, 255, 255, 0.3)';
    });
    
    tag.addEventListener('mouseleave', function() {
        this.style.transform = 'scale(1)';
        this.style.background = 'rgba(255, 255, 255, 0.2)';
    });
});

// App Mockup Interactive Elements
document.querySelectorAll('.result-item').forEach(item => {
    item.addEventListener('mouseenter', function() {
        this.style.background = '#4b5563';
        this.style.transform = 'translateX(5px)';
    });
    
    item.addEventListener('mouseleave', function() {
        this.style.background = '#374151';
        this.style.transform = 'translateX(0)';
    });
});

// Search Bar Animation
const searchBar = document.querySelector('.search-bar');
if (searchBar) {
    let searchText = searchBar.querySelector('span');
    const searchTerms = [
        'Searching "function" in 50,000+ files...',
        'Searching "error" in 25,000+ files...',
        'Searching "class" in 75,000+ files...',
        'Searching "import" in 100,000+ files...'
    ];
    
    let currentIndex = 0;
    
    setInterval(() => {
        searchText.style.opacity = '0';
        setTimeout(() => {
            searchText.textContent = searchTerms[currentIndex];
            searchText.style.opacity = '1';
            currentIndex = (currentIndex + 1) % searchTerms.length;
        }, 300);
    }, 3000);
}

// Contact Form Handler (if you add one later)
function handleContactForm(event) {
    event.preventDefault();
    
    // Get form data
    const formData = new FormData(event.target);
    const data = Object.fromEntries(formData);
    
    // Validate form
    if (!data.email || !data.message) {
        alert('Please fill in all required fields.');
        return;
    }
    
    // Send form data (replace with actual form submission)
    console.log('Contact form submitted:', data);
    alert('Thank you for your message! We\'ll get back to you soon.');
    
    // Reset form
    event.target.reset();
}

// Newsletter Signup Handler (if you add one later)
function handleNewsletterSignup(event) {
    event.preventDefault();
    
    const email = event.target.querySelector('input[type="email"]').value;
    
    if (!email) {
        alert('Please enter your email address.');
        return;
    }
    
    // Add email to newsletter (replace with actual signup logic)
    console.log('Newsletter signup:', email);
    alert('Thank you for subscribing to our newsletter!');
    
    event.target.reset();
}

// Performance Monitoring
window.addEventListener('load', function() {
    // Track page load performance
    const loadTime = performance.now();
    console.log(`Page loaded in ${loadTime.toFixed(2)}ms`);
    
    // Track user engagement
    let scrollDepth = 0;
    window.addEventListener('scroll', function() {
        const scrolled = (window.scrollY / (document.body.scrollHeight - window.innerHeight)) * 100;
        if (scrolled > scrollDepth) {
            scrollDepth = scrolled;
            console.log(`User scrolled ${scrollDepth.toFixed(1)}% of the page`);
        }
    });
});

// Keyboard Navigation Support
document.addEventListener('keydown', function(e) {
    // Escape key closes mobile menu
    if (e.key === 'Escape') {
        const hamburger = document.querySelector('.hamburger');
        const navMenu = document.querySelector('.nav-menu');
        if (hamburger && navMenu) {
            hamburger.classList.remove('active');
            navMenu.classList.remove('active');
        }
    }
    
    // Ctrl/Cmd + K for search (if you add a search feature)
    if ((e.ctrlKey || e.metaKey) && e.key === 'k') {
        e.preventDefault();
        // Focus on search input or open search modal
        console.log('Search shortcut triggered');
    }
});

// Accessibility Improvements
document.addEventListener('DOMContentLoaded', function() {
    // Add focus indicators for keyboard navigation
    const focusableElements = document.querySelectorAll('a, button, input, textarea, select');
    focusableElements.forEach(element => {
        element.addEventListener('focus', function() {
            this.style.outline = '2px solid #2563eb';
            this.style.outlineOffset = '2px';
        });
        
        element.addEventListener('blur', function() {
            this.style.outline = 'none';
        });
    });
    
    // Add ARIA labels for better screen reader support
    const buttons = document.querySelectorAll('button, .btn-primary, .btn-secondary, .btn-download');
    buttons.forEach(button => {
        if (!button.getAttribute('aria-label')) {
            button.setAttribute('aria-label', button.textContent.trim());
        }
    });
});

// Error Handling
window.addEventListener('error', function(e) {
    console.error('JavaScript error:', e.error);
    // You can send this to your error tracking service
});

// Analytics Tracking (replace with your actual analytics)
function trackEvent(eventName, properties = {}) {
    console.log('Event tracked:', eventName, properties);
    // Replace with actual analytics tracking (Google Analytics, Mixpanel, etc.)
}

// Track important user interactions
document.addEventListener('DOMContentLoaded', function() {
    // Track pricing card clicks
    document.querySelectorAll('.pricing-card .btn-primary').forEach(button => {
        button.addEventListener('click', function() {
            const plan = this.closest('.pricing-card').querySelector('h3').textContent;
            trackEvent('pricing_plan_clicked', { plan: plan });
        });
    });
    
    // Track download attempts
    document.querySelectorAll('.btn-download').forEach(button => {
        button.addEventListener('click', function() {
            const platform = this.textContent.includes('Windows') ? 'Windows' : 
                            this.textContent.includes('macOS') ? 'macOS' : 'Linux';
            trackEvent('download_attempted', { platform: platform });
        });
    });
    
    // Track feature card interactions
    document.querySelectorAll('.feature-card').forEach(card => {
        card.addEventListener('click', function() {
            const feature = this.querySelector('h3').textContent;
            trackEvent('feature_card_clicked', { feature: feature });
        });
    });
});
