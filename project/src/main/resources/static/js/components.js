/**
 * Carbook Components Loader
 * Charge dynamiquement les composants réutilisables (navbar, footer, etc.)
 */

const Components = {
    basePath: '/components',
    
    /**
     * Charge un composant HTML dans un élément
     * @param {string} componentName - Nom du composant (sans .html)
     * @param {string} targetSelector - Sélecteur CSS de l'élément cible
     * @param {Function} callback - Fonction appelée après le chargement
     */
    async load(componentName, targetSelector, callback) {
        try {
            const response = await fetch(`${this.basePath}/${componentName}.html`);
            if (!response.ok) throw new Error(`Erreur chargement ${componentName}`);
            const html = await response.text();
            const target = document.querySelector(targetSelector);
            if (target) {
                target.innerHTML = html;
                if (callback) callback();
            }
        } catch (error) {
            console.error(`Erreur lors du chargement de ${componentName}:`, error);
        }
    },

    /**
     * Charge les scripts communs dynamiquement
     */
    async loadScripts() {
        try {
            const response = await fetch(`${this.basePath}/scripts.html`);
            if (!response.ok) throw new Error('Erreur chargement scripts');
            const html = await response.text();
            
            // Parse les scripts et les ajoute au DOM
            const temp = document.createElement('div');
            temp.innerHTML = html;
            const scripts = temp.querySelectorAll('script');
            
            for (const script of scripts) {
                await this.loadScript(script.src);
            }
        } catch (error) {
            console.error('Erreur lors du chargement des scripts:', error);
        }
    },

    /**
     * Charge un script de manière synchrone
     */
    loadScript(src) {
        return new Promise((resolve, reject) => {
            const script = document.createElement('script');
            script.src = src;
            script.onload = resolve;
            script.onerror = reject;
            document.body.appendChild(script);
        });
    },

    /**
     * Active le lien de navigation correspondant à la page actuelle
     */
    setActiveNavLink() {
        const path = window.location.pathname;
        const navLinks = document.querySelectorAll('#ftco-nav .nav-link');
        
        navLinks.forEach(link => {
            link.parentElement.classList.remove('active');
            const href = link.getAttribute('href');
            
            // Correspondance exacte ou page d'accueil
            if (href === path || 
                (path === '/' && href === '/') ||
                (path === '/project' && href === '/') ||
                (path.startsWith(href) && href !== '/')) {
                link.parentElement.classList.add('active');
            }
        });
    },

    /**
     * Initialise tous les composants de la page
     */
    async init() {
        // Charger la navbar
        await this.load('navbar', '#navbar-placeholder', () => {
            this.setActiveNavLink();
        });
        
        // Charger le footer
        await this.load('footer', '#footer-placeholder');
    }
};

// Auto-initialisation au chargement du DOM
document.addEventListener('DOMContentLoaded', () => {
    Components.init();
});

