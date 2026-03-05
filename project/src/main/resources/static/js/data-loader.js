/**
 * Carbook Dynamic Data Loader
 * Charge les données depuis l'API et les affiche dynamiquement
 */

const CarbookData = {
    basePath: '/api',

    /**
     * Effectue une requête GET vers l'API
     */
    async fetch(endpoint) {
        try {
            const response = await fetch(`${this.basePath}${endpoint}`);
            if (!response.ok) throw new Error(`Erreur API: ${response.status}`);
            return await response.json();
        } catch (error) {
            console.error(`Erreur lors du chargement de ${endpoint}:`, error);
            return null;
        }
    },

    /**
     * Charge et affiche les voitures featured dans le carousel
     */
    async loadFeaturedCars() {
        const container = document.querySelector('.carousel-car');
        if (!container) return;

        const voitures = await this.fetch('/voitures/featured');
        if (!voitures || voitures.length === 0) return;

        container.innerHTML = voitures.map(v => `
            <div class="item">
                <div class="car-wrap rounded ftco-animate">
                    <div class="img rounded d-flex align-items-end" style="background-image: url(${v.image});"></div>
                    <div class="text">
                        <h2 class="mb-0"><a href="/car-single?id=${v.id}">${v.nom}</a></h2>
                        <div class="d-flex mb-3">
                            <span class="cat">${v.marque ? v.marque.nom : ''}</span>
                            <p class="price ml-auto">$${v.prixJour} <span>/day</span></p>
                        </div>
                        <p class="d-flex mb-0 d-block">
                            <a href="/car-single?id=${v.id}" class="btn btn-primary py-2 mr-1">Book now</a> 
                            <a href="/car-single?id=${v.id}" class="btn btn-secondary py-2 ml-1">Details</a>
                        </p>
                    </div>
                </div>
            </div>
        `).join('');

        // Réinitialiser le carousel Owl
        if (typeof $.fn.owlCarousel !== 'undefined') {
            $(container).owlCarousel('destroy');
            $(container).owlCarousel({
                center: false,
                loop: true,
                autoplay: true,
                items: 3,
                margin: 30,
                stagePadding: 0,
                nav: false,
                navText: ['<span class="ion-ios-arrow-back">', '<span class="ion-ios-arrow-forward">'],
                responsive: {
                    0: { items: 1 },
                    600: { items: 2 },
                    1000: { items: 3 }
                }
            });
        }
    },

    /**
     * Charge et affiche toutes les voitures dans la page cars
     */
    async loadAllCars() {
        const container = document.getElementById('cars-container');
        if (!container) return;

        const voitures = await this.fetch('/voitures');
        if (!voitures || voitures.length === 0) {
            container.innerHTML = '<p class="text-center">Aucune voiture disponible</p>';
            return;
        }

        container.innerHTML = voitures.map(v => `
            <div class="col-md-4">
                <div class="car-wrap rounded ftco-animate">
                    <div class="img rounded d-flex align-items-end" style="background-image: url(${v.image});">
                    </div>
                    <div class="text">
                        <h2 class="mb-0"><a href="/car-single?id=${v.id}">${v.nom}</a></h2>
                        <div class="d-flex mb-3">
                            <span class="cat">${v.marque ? v.marque.nom : ''}</span>
                            <p class="price ml-auto">$${v.prixJour} <span>/day</span></p>
                        </div>
                        <p class="d-flex mb-0 d-block">
                            <a href="/car-single?id=${v.id}" class="btn btn-primary py-2 mr-1">Book now</a> 
                            <a href="/car-single?id=${v.id}" class="btn btn-secondary py-2 ml-1">Details</a>
                        </p>
                    </div>
                </div>
            </div>
        `).join('');
    },

    /**
     * Charge et affiche les détails d'une voiture
     */
    async loadCarDetails() {
        const container = document.getElementById('car-details');
        if (!container) return;

        const urlParams = new URLSearchParams(window.location.search);
        const id = urlParams.get('id');
        if (!id) return;

        const voiture = await this.fetch(`/voiture?id=${id}`);
        if (!voiture) {
            container.innerHTML = '<p class="text-center">Voiture non trouvée</p>';
            return;
        }

        document.getElementById('car-name')?.textContent && (document.getElementById('car-name').textContent = voiture.nom);
        document.getElementById('car-image')?.style && (document.getElementById('car-image').style.backgroundImage = `url(${voiture.image})`);
        document.getElementById('car-price')?.textContent && (document.getElementById('car-price').textContent = `$${voiture.prixJour}/day`);
        document.getElementById('car-brand')?.textContent && (document.getElementById('car-brand').textContent = voiture.marque?.nom || '');
        document.getElementById('car-description')?.textContent && (document.getElementById('car-description').textContent = voiture.description || '');
        document.getElementById('car-year')?.textContent && (document.getElementById('car-year').textContent = voiture.annee);
        document.getElementById('car-seats')?.textContent && (document.getElementById('car-seats').textContent = voiture.places);
        document.getElementById('car-doors')?.textContent && (document.getElementById('car-doors').textContent = voiture.portes);
        document.getElementById('car-transmission')?.textContent && (document.getElementById('car-transmission').textContent = voiture.transmission);
        document.getElementById('car-fuel')?.textContent && (document.getElementById('car-fuel').textContent = voiture.carburant);
    },

    /**
     * Charge et affiche les services
     */
    async loadServices() {
        const container = document.getElementById('services-container');
        if (!container) return;

        const services = await this.fetch('/services');
        if (!services || services.length === 0) return;

        container.innerHTML = services.map(s => `
            <div class="col-md-3">
                <div class="services services-2 w-100 text-center">
                    <div class="icon d-flex align-items-center justify-content-center">
                        <span class="${s.icone}"></span>
                    </div>
                    <div class="text w-100">
                        <h3 class="heading mb-2">${s.titre}</h3>
                        <p>${s.description}</p>
                    </div>
                </div>
            </div>
        `).join('');
    },

    /**
     * Charge et affiche les témoignages
     */
    async loadTestimonials() {
        const container = document.querySelector('.carousel-testimony');
        if (!container) return;

        const temoignages = await this.fetch('/temoignages');
        if (!temoignages || temoignages.length === 0) return;

        container.innerHTML = temoignages.map(t => `
            <div class="item">
                <div class="testimony-wrap rounded text-center py-4 pb-5">
                    <div class="user-img mb-2" style="background-image: url(${t.photo})"></div>
                    <div class="text pt-4">
                        <p class="mb-4">${t.commentaire}</p>
                        <p class="name">${t.nom}</p>
                        <span class="position">${t.poste}</span>
                    </div>
                </div>
            </div>
        `).join('');

        // Réinitialiser le carousel
        if (typeof $.fn.owlCarousel !== 'undefined') {
            $(container).owlCarousel('destroy');
            $(container).owlCarousel({
                center: false,
                loop: true,
                autoplay: true,
                items: 3,
                margin: 30,
                stagePadding: 0,
                nav: false,
                responsive: {
                    0: { items: 1 },
                    600: { items: 2 },
                    1000: { items: 3 }
                }
            });
        }
    },

    /**
     * Charge et affiche les articles de blog récents
     */
    async loadRecentBlogs() {
        const container = document.getElementById('blogs-container');
        if (!container) return;

        const blogs = await this.fetch('/blogs/recent');
        if (!blogs || blogs.length === 0) return;

        container.innerHTML = blogs.map(b => `
            <div class="col-md-4 d-flex ftco-animate">
                <div class="blog-entry justify-content-end">
                    <a href="/blog-single?id=${b.id}" class="block-20" style="background-image: url('${b.image}');"></a>
                    <div class="text pt-4">
                        <div class="meta mb-3">
                            <div><a href="#">${new Date(b.datePublication).toLocaleDateString('en-US', { month: 'short', day: 'numeric', year: 'numeric' })}</a></div>
                            <div><a href="#">${b.auteur}</a></div>
                            <div><a href="#" class="meta-chat"><span class="icon-chat"></span> ${b.commentairesCount}</a></div>
                        </div>
                        <h3 class="heading mt-2"><a href="/blog-single?id=${b.id}">${b.titre}</a></h3>
                        <p><a href="/blog-single?id=${b.id}" class="btn btn-primary">Read more</a></p>
                    </div>
                </div>
            </div>
        `).join('');
    },

    /**
     * Charge et affiche tous les articles de blog
     */
    async loadAllBlogs() {
        const container = document.getElementById('all-blogs-container');
        if (!container) return;

        const blogs = await this.fetch('/blogs');
        if (!blogs || blogs.length === 0) {
            container.innerHTML = '<p class="text-center">Aucun article disponible</p>';
            return;
        }

        container.innerHTML = blogs.map(b => `
            <div class="col-md-4 d-flex ftco-animate">
                <div class="blog-entry justify-content-end">
                    <a href="/blog-single?id=${b.id}" class="block-20" style="background-image: url('${b.image}');"></a>
                    <div class="text pt-4">
                        <div class="meta mb-3">
                            <div><a href="#">${new Date(b.datePublication).toLocaleDateString('en-US', { month: 'short', day: 'numeric', year: 'numeric' })}</a></div>
                            <div><a href="#">${b.auteur}</a></div>
                            <div><a href="#" class="meta-chat"><span class="icon-chat"></span> ${b.commentairesCount}</a></div>
                        </div>
                        <h3 class="heading mt-2"><a href="/blog-single?id=${b.id}">${b.titre}</a></h3>
                        <p>${b.extrait}</p>
                        <p><a href="/blog-single?id=${b.id}" class="btn btn-primary">Read more</a></p>
                    </div>
                </div>
            </div>
        `).join('');
    },

    /**
     * Charge et affiche les statistiques
     */
    async loadStatistics() {
        const container = document.getElementById('stats-container');
        if (!container) return;

        const stats = await this.fetch('/statistiques');
        if (!stats || stats.length === 0) return;

        const labels = {
            'years_experience': 'Year <br>Experienced',
            'total_cars': 'Total <br>Cars',
            'happy_customers': 'Happy <br>Customers',
            'total_branches': 'Total <br>Branches'
        };

        container.innerHTML = stats.map((s, i) => `
            <div class="col-md-6 col-lg-3 justify-content-center counter-wrap ftco-animate">
                <div class="block-18">
                    <div class="text ${i < stats.length - 1 ? 'text-border' : ''} d-flex align-items-center">
                        <strong class="number" data-number="${s.valeur}">0</strong>
                        <span>${labels[s.cle] || s.label}</span>
                    </div>
                </div>
            </div>
        `).join('');

        // Réinitialiser le compteur animé
        if (typeof $.fn.animateNumber !== 'undefined') {
            $('.number').each(function() {
                const $this = $(this);
                const num = $this.data('number');
                $this.animateNumber({ number: num }, 2000);
            });
        }
    },

    /**
     * Initialise le chargement des données selon la page
     */
    async init() {
        // Charger les données en fonction de la page actuelle
        const path = window.location.pathname;

        // Page d'accueil
        if (path === '/' || path === '/project' || path.endsWith('/index')) {
            await Promise.all([
                this.loadFeaturedCars(),
                this.loadServices(),
                this.loadTestimonials(),
                this.loadRecentBlogs(),
                this.loadStatistics()
            ]);
        }

        // Page Cars
        if (path.includes('/car') && !path.includes('/car-single')) {
            await this.loadAllCars();
        }

        // Page Car Single
        if (path.includes('/car-single')) {
            await this.loadCarDetails();
        }

        // Page Services
        if (path.includes('/services')) {
            await this.loadServices();
        }

        // Page Blog
        if (path.includes('/blog') && !path.includes('/blog-single')) {
            await this.loadAllBlogs();
        }

        // Page About
        if (path.includes('/about')) {
            await Promise.all([
                this.loadTestimonials(),
                this.loadStatistics()
            ]);
        }
    }
};

// Auto-initialisation après le chargement du DOM et des composants
document.addEventListener('DOMContentLoaded', () => {
    // Attendre un peu que les autres scripts soient chargés
    setTimeout(() => {
        CarbookData.init();
    }, 500);
});

