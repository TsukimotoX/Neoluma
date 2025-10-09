document.addEventListener('DOMContentLoaded', function() {
    const navLinks = document.querySelectorAll('a[href^="#"]');
    navLinks.forEach(link => {
        link.addEventListener('click', function(e) {
            e.preventDefault();
            const targetId = this.getAttribute('href');
            const targetSection = document.querySelector(targetId);
            if (targetSection) {
                const headerOffset = 80;
                const elementPosition = targetSection.getBoundingClientRect().top;
                const offsetPosition = elementPosition + window.pageYOffset - headerOffset;

                window.scrollTo({
                    top: offsetPosition,
                    behavior: 'smooth'
                });
            }
        });
    });

    const mobileMenuBtn = document.querySelector('.mobile-menu-btn');
    const navLinks_mobile = document.querySelector('.nav-links');
    
    if (mobileMenuBtn) {
        mobileMenuBtn.addEventListener('click', function() {
            navLinks_mobile.classList.toggle('active');
        });
    }

    window.addEventListener('scroll', function() {
        const header = document.querySelector('.header');
        if (window.scrollY > 100) {
            header.style.background = 'rgba(15, 23, 42, 0.98)';
        } else {
            header.style.background = 'rgba(15, 23, 42, 0.95)';
        }
    });

    const syntaxTabs = document.querySelectorAll('.syntax-tab');
    const syntaxPanels = document.querySelectorAll('.syntax-panel');

    syntaxTabs.forEach(tab => {
        tab.addEventListener('click', function() {
            const targetPanel = this.getAttribute('data-tab');
            
            syntaxTabs.forEach(t => t.classList.remove('active'));
            syntaxPanels.forEach(p => p.classList.remove('active'));
            
            this.classList.add('active');
            const panel = document.querySelector(`[data-panel="${targetPanel}"]`);
            if (panel) {
                panel.classList.add('active');
            }
        });
    });

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

    const animateElements = document.querySelectorAll('.feature-card, .type-card, .directive-card, .timeline-item, .community-card');
    animateElements.forEach(el => {
        el.style.opacity = '0';
        el.style.transform = 'translateY(20px)';
        el.style.transition = 'opacity 0.6s ease, transform 0.6s ease';
        observer.observe(el);
    });

    const codeBlocks = document.querySelectorAll('pre code');
    codeBlocks.forEach(block => {
        const pre = block.parentElement;
        const copyBtn = document.createElement('button');
        copyBtn.innerHTML = '<i class="fas fa-copy"></i>';
        copyBtn.className = 'copy-btn';
        copyBtn.style.cssText = `
            position: absolute;
            top: 1rem;
            right: 1rem;
            background: rgba(51, 65, 85, 0.8);
            border: 1px solid rgba(148, 163, 184, 0.3);
            color: #94a3b8;
            padding: 0.5rem;
            border-radius: 0.5rem;
            cursor: pointer;
            opacity: 0;
            transition: all 0.3s ease;
            z-index: 10;
        `;
        
        pre.style.position = 'relative';
        pre.appendChild(copyBtn);
        
        pre.addEventListener('mouseenter', () => {
            copyBtn.style.opacity = '1';
        });
        
        pre.addEventListener('mouseleave', () => {
            copyBtn.style.opacity = '0';
        });
        
        copyBtn.addEventListener('click', async () => {
            try {
                await navigator.clipboard.writeText(block.textContent);
                copyBtn.innerHTML = '<i class="fas fa-check"></i>';
                copyBtn.style.color = '#10b981';
                setTimeout(() => {
                    copyBtn.innerHTML = '<i class="fas fa-copy"></i>';
                    copyBtn.style.color = '#94a3b8';
                }, 2000);
            } catch (err) {
                console.error('Failed to copy code:', err);
            }
        });
    });

    window.addEventListener('scroll', function() {
        const scrolled = window.pageYOffset;
        const parallax1 = document.querySelector('.element-1');
        const parallax2 = document.querySelector('.element-2');
        const parallax3 = document.querySelector('.element-3');
        
        if (parallax1) parallax1.style.transform = `translateY(${scrolled * 0.1}px) rotate(${scrolled * 0.02}deg)`;
        if (parallax2) parallax2.style.transform = `translateY(${scrolled * -0.1}px) rotate(${scrolled * -0.02}deg)`;
        if (parallax3) parallax3.style.transform = `translateY(${scrolled * 0.05}px) rotate(${scrolled * 0.01}deg)`;
    });


    const mainElements = document.querySelectorAll('.main-text > *');
    mainElements.forEach((el, index) => {
        el.style.opacity = '0';
        el.style.transform = 'translateY(30px)';
        el.style.animation = `fadeInUp 0.8s ease forwards ${index * 0.1}s`;
    });

    const style = document.createElement('style');
    style.textContent = `
        @keyframes fadeInUp {
            to {
                opacity: 1;
                transform: translateY(0);
            }
        }
        
        .nav-links.active {
            display: flex;
            flex-direction: column;
            position: absolute;
            top: 100%;
            left: 0;
            right: 0;
            background: rgba(15, 23, 42, 0.98);
            padding: 1rem;
            border-top: 1px solid rgba(51, 65, 85, 0.3);
        }
        
        @media (max-width: 768px) {
            .nav-links {
                display: none;
            }
        }
    `;
    document.head.appendChild(style);
});


if (typeof Prism !== 'undefined') {
    Prism.languages.neoluma = {
        'comment': {
            pattern: /\/\/.*$/m,
            greedy: true
        },
        'annotation': {
            pattern: /@\w+/,
            alias: 'keyword'
        },
        'directive': {
            pattern: /#\w+/,
            alias: 'keyword'
        },
        'keyword': /\b(?:fn|let|mut|if|else|for|while|return|match|enum|async|await|unsafe|comptime|entry|override)\b/,
        'type': /\b(?:int|float|bool|string|array|dict|set|result|void|number)\b/,
        'function': /\b\w+(?=\s*\()/,
        'string': {
            pattern: /"(?:[^"\\]|\\.)*"/,
            greedy: true
        },
        'interpolation': {
            pattern: /\{[^}]+\}/,
            inside: {
                'punctuation': /[{}]/,
                'expression': /.+/
            }
        },
        'number': /\b\d+(?:\.\d+)?\b/,
        'boolean': /\b(?:true|false)\b/,
        'operator': /[+\-*/%=!<>&|^~?:]/,
        'punctuation': /[{}\[\]();,.]/
    };
}