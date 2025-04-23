#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

/*
OUTPUT EXAMPLE

{
  "page": 1,
  "results": [
    {
      "adult": false,
      "backdrop_path": "/4ngUkStgauKB9L0Jjwkx1Hz1dry.jpg",
      "genre_ids": [
        53,
        18
      ],
      "id": 4553,
      "original_language": "en",
      "original_title": "The Machinist",
      "overview": "Trevor Reznik, non riesce a dormire da un anno; il suo fisico, stremato dalla mancanza di riposo e di energie è ormai spettrale, cadaverico. Per Trevor, operaio in fabbrica, lavorare è ormai diventato un'impresa impossibile, ogni giorno più difficile. A peggiorare il tutto accade un grave incidente, causato da una sua disattenzione, che quasi costa la vita di un collega. Trevor, pallido e schivo, raggiunge un livello di alienazione dal mondo che lo trasforma lentamente in un fantasma delirante: non ha rapporti con nessuno, tanto meno con i suoi colleghi, dato che dopo l'incidente sospetta che qualcuno abbia complottato contro di lui per farlo licenziare. Le uniche due persone con le quali Trevor ha ancora un rapporto e alle quali è legato sono Stevie, una prostituta divenuta nel tempo sua confidente e Marie la cameriera del bar dell'aeroporto nel quale ogni notte inspiegabilmente Trevor si reca per bere un caffè. Verso la fine la causa della sua insonnia lentamente riaffiorerà.",
      "popularity": 13.5092,
      "poster_path": "/AjiSHn5CivzzPdHxtP4oqSTasv8.jpg",
      "release_date": "2004-09-24",
      "title": "L'uomo senza sonno",
      "video": false,
      "vote_average": 7.527,
      "vote_count": 5704
    }
  ],
  "total_pages": 1,
  "total_results": 1
}
*/
extern json search_movie(const std::string& title, const std::string& language);

/*
EXAMPLE OUTPUT

{
  "page": 1,
  "results": [
    {
      "adult": false,
      "backdrop_path": "/7dowXHcFccjmxf0YZYxDFkfVq65.jpg",
      "genre_ids": [
        18
      ],
      "id": 100088,
      "origin_country": [
        "US"
      ],
      "original_language": "en",
      "original_name": "The Last of Us",
      "overview": "Vent’anni dopo la distruzione della civiltà moderna Joel, uno scaltro sopravvissuto, viene incaricato di far uscire Ellie, una ragazza di 14 anni, da una zona di quarantena sotto stretta sorveglianza. Un compito all’apparenza facile che si trasforma presto in un viaggio brutale e straziante, in cui i due si troveranno a dover attraversare gli Stati Uniti insieme e a dipendere l’uno dall’altra per sopravvivere.",
      "popularity": 519.1289,
      "poster_path": "/6WlffOd3bszyndGgsKGcUL6k28n.jpg",
      "first_air_date": "2023-01-15",
      "name": "The Last of Us",
      "vote_average": 8.581,
      "vote_count": 5643
    },
    ...
  ],
  "total_pages": 1,
  "total_results": 3
}
*/
extern json search_tv_show(const std::string& title, const std::string& language);

/*
EXAMPLE OUTPUT

{
  "adult": false,
  "backdrop_path": "/4ngUkStgauKB9L0Jjwkx1Hz1dry.jpg",
  "belongs_to_collection": null,
  "budget": 5000000,
  "genres": [
    {
      "id": 53,
      "name": "Thriller"
    },
    {
      "id": 18,
      "name": "Dramma"
    }
  ],
  "homepage": "",
  "id": 4553,
  "imdb_id": "tt0361862",
  "origin_country": [
    "US"
  ],
  "original_language": "en",
  "original_title": "The Machinist",
  "overview": "Trevor Reznik, non riesce a dormire da un anno; il suo fisico, stremato dalla mancanza di riposo e di energie è ormai spettrale, cadaverico. Per Trevor, operaio in fabbrica, lavorare è ormai diventato un'impresa impossibile, ogni giorno più difficile. A peggiorare il tutto accade un grave incidente, causato da una sua disattenzione, che quasi costa la vita di un collega. Trevor, pallido e schivo, raggiunge un livello di alienazione dal mondo che lo trasforma lentamente in un fantasma delirante: non ha rapporti con nessuno, tanto meno con i suoi colleghi, dato che dopo l'incidente sospetta che qualcuno abbia complottato contro di lui per farlo licenziare. Le uniche due persone con le quali Trevor ha ancora un rapporto e alle quali è legato sono Stevie, una prostituta divenuta nel tempo sua confidente e Marie la cameriera del bar dell'aeroporto nel quale ogni notte inspiegabilmente Trevor si reca per bere un caffè. Verso la fine la causa della sua insonnia lentamente riaffiorerà.",
  "popularity": 13.5092,
  "poster_path": "/AjiSHn5CivzzPdHxtP4oqSTasv8.jpg",
  "production_companies": [
    {
      "id": 7956,
      "logo_path": null,
      "name": "Castelao Productions",
      "origin_country": "ES"
    },
    {
      "id": 3631,
      "logo_path": "/ttrj8O0KF5PbUoHamU2mTOLB6d9.png",
      "name": "Filmax",
      "origin_country": "ES"
    }
  ],
  "production_countries": [
    {
      "iso_3166_1": "ES",
      "name": "Spain"
    }
  ],
  "release_date": "2004-09-24",
  "revenue": 8203235,
  "runtime": 102,
  "spoken_languages": [
    {
      "english_name": "English",
      "iso_639_1": "en",
      "name": "English"
    }
  ],
  "status": "Released",
  "tagline": "Se fossi più magro non esisteresti",
  "title": "L'uomo senza sonno",
  "video": false,
  "vote_average": 7.527,
  "vote_count": 5703
}
*/
extern json get_movie_details(int id, const std::string& language);

/*
EXAMPLE OUTPUT

{
  "adult": false,
  "backdrop_path": "/7dowXHcFccjmxf0YZYxDFkfVq65.jpg",
  "created_by": [
    {
      "id": 35796,
      "credit_id": "5e84f06a3344c600153f6a57",
      "name": "Craig Mazin",
      "original_name": "Craig Mazin",
      "gender": 2,
      "profile_path": "/uEhna6qcMuyU5TP7irpTUZ2ZsZc.jpg"
    },
    {
      "id": 1295692,
      "credit_id": "5e84f03598f1f10016a985c0",
      "name": "Neil Druckmann",
      "original_name": "Neil Druckmann",
      "gender": 2,
      "profile_path": "/bVUsM4aYiHbeSYE1xAw2H5Z1ANU.jpg"
    }
  ],
  "episode_run_time": [],
  "first_air_date": "2023-01-15",
  "genres": [
    {
      "id": 18,
      "name": "Dramma"
    }
  ],
  "homepage": "https://www.hbo.com/the-last-of-us",
  "id": 100088,
  "in_production": true,
  "languages": [
    "en"
  ],
  "last_air_date": "2025-04-20",
  "last_episode_to_air": {
    "id": 5517229,
    "name": "Episodio 2",
    "overview": "",
    "vote_average": 7.917,
    "vote_count": 12,
    "air_date": "2025-04-20",
    "episode_number": 2,
    "episode_type": "standard",
    "production_code": "",
    "runtime": 57,
    "season_number": 2,
    "show_id": 100088,
    "still_path": "/nJOU5Ok1VdJGSrzTYwttRxHOCkJ.jpg"
  },
  "name": "The Last of Us",
  "next_episode_to_air": {
    "id": 5994269,
    "name": "Episodio 3",
    "overview": "",
    "vote_average": 0,
    "vote_count": 0,
    "air_date": "2025-04-27",
    "episode_number": 3,
    "episode_type": "standard",
    "production_code": "",
    "runtime": 60,
    "season_number": 2,
    "show_id": 100088,
    "still_path": null
  },
  "networks": [
    {
      "id": 49,
      "logo_path": "/tuomPhY2UtuPTqqFnKMVHvSb724.png",
      "name": "HBO",
      "origin_country": "US"
    }
  ],
  "number_of_episodes": 16,
  "number_of_seasons": 2,
  "origin_country": [
    "US"
  ],
  "original_language": "en",
  "original_name": "The Last of Us",
  "overview": "Vent’anni dopo la distruzione della civiltà moderna Joel, uno scaltro sopravvissuto, viene incaricato di far uscire Ellie, una ragazza di 14 anni, da una zona di quarantena sotto stretta sorveglianza. Un compito all’apparenza facile che si trasforma presto in un viaggio brutale e straziante, in cui i due si troveranno a dover attraversare gli Stati Uniti insieme e a dipendere l’uno dall’altra per sopravvivere.",
  "popularity": 519.1289,
  "poster_path": "/6WlffOd3bszyndGgsKGcUL6k28n.jpg",
  "production_companies": [
    {
      "id": 125281,
      "logo_path": "/3hV8pyxzAJgEjiSYVv1WZ0ZYayp.png",
      "name": "PlayStation Productions",
      "origin_country": "US"
    },
    {
      "id": 11073,
      "logo_path": "/aCbASRcI1MI7DXjPbSW9Fcv9uGR.png",
      "name": "Sony Pictures Television",
      "origin_country": "US"
    },
    {
      "id": 23217,
      "logo_path": "/kXBZdQigEf6QiTLzo6TFLAa7jKD.png",
      "name": "Naughty Dog",
      "origin_country": "US"
    },
    {
      "id": 119645,
      "logo_path": null,
      "name": "Word Games",
      "origin_country": "US"
    },
    {
      "id": 115241,
      "logo_path": null,
      "name": "The Mighty Mint",
      "origin_country": "US"
    },
    {
      "id": 3268,
      "logo_path": "/tuomPhY2UtuPTqqFnKMVHvSb724.png",
      "name": "HBO",
      "origin_country": "US"
    }
  ],
  "production_countries": [
    {
      "iso_3166_1": "US",
      "name": "United States of America"
    }
  ],
  "seasons": [
    {
      "air_date": "2023-01-15",
      "episode_count": 9,
      "id": 144593,
      "name": "Stagione 1",
      "overview": "Da Boston a Salt Lake City, attraverso un mondo devastato da una piaga apocalittica: un uomo e una ragazza viaggiano per tentare di salvare l'umanità.",
      "poster_path": "/bSWFzaboUFxfxBE1pIWlRsp5TvE.jpg",
      "season_number": 1,
      "vote_average": 7.9
    },
    {
      "air_date": "2025-04-13",
      "episode_count": 7,
      "id": 405376,
      "name": "Stagione 2",
      "overview": "Cinque anni dopo gli eventi della prima stagione, Joel ed Ellie saranno trascinati in un conflitto fra di loro e contro un mondo persino più pericoloso e imprevedibile di quello che si erano lasciati alle spalle.",
      "poster_path": "/2IXXPbfqGX6HUHD6HX2z56g2ra.jpg",
      "season_number": 2,
      "vote_average": 7.6
    }
  ],
  "spoken_languages": [
    {
      "english_name": "English",
      "iso_639_1": "en",
      "name": "English"
    }
  ],
  "status": "Returning Series",
  "tagline": "Quando ti perdi nell'oscurità, cerca la luce",
  "type": "Scripted",
  "vote_average": 8.581,
  "vote_count": 5643
}
*/
extern json get_tv_show_details(int id, const std::string& language);

/*
EXAMPLE OUTPUT

{
  "_id": "5e614cd3357c00001631a6ef",
  "air_date": "2023-01-15",
  "episodes": [
    {
      "air_date": "2023-01-15",
      "episode_number": 1,
      "episode_type": "standard",
      "id": 2181581,
      "name": "Quando sei perso nell'oscurità",
      "overview": "Dopo che una pandemia ha distrutto la civiltà, un sopravvissuto si prende cura di una quattordicenne che potrebbe essere l'ultima speranza dell'umanità.",
      "production_code": "",
      "runtime": 81,
      "season_number": 1,
      "show_id": 100088,
      "still_path": "/3VeY1k8wFyhcrMyQ8jpGegh9beU.jpg",
      "vote_average": 8.405,
      "vote_count": 227,
      "crew": [
        {
          "job": "Writer",
          "department": "Writing",
          "credit_id": "619c3707e2ff32004377d7cd",
          "adult": false,
          "gender": 2,
          "id": 1295692,
          "known_for_department": "Acting",
          "name": "Neil Druckmann",
          "original_name": "Neil Druckmann",
          "popularity": 0.5709,
          "profile_path": "/bVUsM4aYiHbeSYE1xAw2H5Z1ANU.jpg"
        },
        {
          "job": "Writer",
          "department": "Writing",
          "credit_id": "619c370063536a00619a08ee",
          "adult": false,
          "gender": 2,
          "id": 35796,
          "known_for_department": "Writing",
          "name": "Craig Mazin",
          "original_name": "Craig Mazin",
          "popularity": 0.6695,
          "profile_path": "/uEhna6qcMuyU5TP7irpTUZ2ZsZc.jpg"
        },
        {
          "job": "Director",
          "department": "Directing",
          "credit_id": "63be15bb813831008046aae9",
          "adult": false,
          "gender": 2,
          "id": 35796,
          "known_for_department": "Writing",
          "name": "Craig Mazin",
          "original_name": "Craig Mazin",
          "popularity": 0.6695,
          "profile_path": "/uEhna6qcMuyU5TP7irpTUZ2ZsZc.jpg"
        },
        {
          "job": "Stunts",
          "department": "Crew",
          "credit_id": "63c4d828595a5600bbe197bb",
          "adult": false,
          "gender": 2,
          "id": 113206,
          "known_for_department": "Crew",
          "name": "Jeff Sanca",
          "original_name": "Jeff Sanca",
          "popularity": 0.3588,
          "profile_path": "/b8s7kVZDoKrUli8rTKxramMtHXd.jpg"
        },
        ...
      ]
    },
    ...
  ],
  "name": "Stagione 1",
  "overview": "Da Boston a Salt Lake City, attraverso un mondo devastato da una piaga apocalittica: un uomo e una ragazza viaggiano per tentare di salvare l'umanità.",
  "id": 144593,
  "poster_path": "/bSWFzaboUFxfxBE1pIWlRsp5TvE.jpg",
  "season_number": 1,
  "vote_average": 7.9
}
*/
extern json get_tv_show_season(int tv_id, int season_number, const std::string& language);

/*
EXAMPLE OUTPUT

{
  "id": 4553,
  "cast": [
    {
      "adult": false,
      "gender": 2,
      "id": 3894,
      "known_for_department": "Acting",
      "name": "Christian Bale",
      "original_name": "Christian Bale",
      "popularity": 2.4328,
      "profile_path": "/7Pxez9J8fuPd2Mn9kex13YALrCQ.jpg",
      "cast_id": 1,
      "character": "Trevor Reznik",
      "credit_id": "52fe43cbc3a36847f8070491",
      "order": 0
    },
    ...
    ],
  "crew": [
    {
      "adult": false,
      "gender": 2,
      "id": 37948,
      "known_for_department": "Directing",
      "name": "Brad Anderson",
      "original_name": "Brad Anderson",
      "popularity": 0.5918,
      "profile_path": "/4XCOsnXocIsOTLdVjktj3gqTu2b.jpg",
      "credit_id": "52fe43cbc3a36847f80704ab",
      "department": "Directing",
      "job": "Director"
    },
    ...
  ]
}
*/
extern json get_movie_cast(int movie_id); // Includes the director

/*
EXAMPLE OUTPUT

{
  "cast": [
    {
      "adult": false,
      "gender": 2,
      "id": 1253360,
      "known_for_department": "Acting",
      "name": "Pedro Pascal",
      "original_name": "Pedro Pascal",
      "popularity": 20.8262,
      "profile_path": "/9VYK7oxcqhjd5LAH6ZFJ3XzOlID.jpg",
      "character": "Joel Miller",
      "credit_id": "6024a814c0ae36003d59cc3c",
      "order": 0
    },
    {
      "adult": false,
      "gender": 3,
      "id": 1668004,
      "known_for_department": "Acting",
      "name": "Bella Ramsey",
      "original_name": "Bella Ramsey",
      "popularity": 10.1834,
      "profile_path": "/4W2iNEfnFIB2TXa8kVgKzNXIwqb.jpg",
      "character": "Ellie Williams",
      "credit_id": "60249992abf8e2003fa458b0",
      "order": 1
    },
    {
      "adult": false,
      "gender": 2,
      "id": 111016,
      "known_for_department": "Acting",
      "name": "Gabriel Luna",
      "original_name": "Gabriel Luna",
      "popularity": 1.5338,
      "profile_path": "/wZ39o38IaEJIfQglF5ovI7lO3ba.jpg",
      "character": "Tommy Miller",
      "credit_id": "6407840b158c85007c1ea4a9",
      "order": 562
    },
    {
      "adult": false,
      "gender": 1,
      "id": 1428070,
      "known_for_department": "Acting",
      "name": "Isabela Merced",
      "original_name": "Isabela Merced",
      "popularity": 11.6145,
      "profile_path": "/zfLScjMHJWkGMzvDZmb1tpsBHuk.jpg",
      "character": "Dina",
      "credit_id": "67fc6f983110bd82dfad1aa8",
      "order": 563
    },
    {
      "adult": false,
      "gender": 2,
      "id": 2569691,
      "known_for_department": "Acting",
      "name": "Young Mazino",
      "original_name": "Young Mazino",
      "popularity": 0.8317,
      "profile_path": "/bseVfwPNhDlToTIGs7FuKzd0uxi.jpg",
      "character": "Jesse",
      "credit_id": "67fc6fa4aacf7cfb26996a94",
      "order": 564
    }
  ],
  "id": 100088
}
*/
extern json get_tv_show_cast(int tv_id); 

/*
EXAMPLE OUTPUT

{
  "adult": false,
  "also_known_as": [
    "Alexander Pascal",
    "Pedro Balmaceda",
    "José Pedro Balmaceda Pascal",
    "Pablo Pascal"
  ],
  "biography": "",
  "birthday": "1975-04-02",
  "deathday": null,
  "gender": 2,
  "homepage": null,
  "id": 1253360,
  "imdb_id": "nm0050959",
  "known_for_department": "Acting",
  "name": "Pedro Pascal",
  "place_of_birth": "Santiago, Chile",
  "popularity": 20.8262,
  "profile_path": "/9VYK7oxcqhjd5LAH6ZFJ3XzOlID.jpg"
}
*/
extern json get_person_details(int person_id, const std::string& language); 

/*
EXAMPLE OUTPUT

{
  "genres": [
    {
      "id": 10759,
      "name": "Action & Adventure"
    },
    {
      "id": 16,
      "name": "Animazione"
    },
    ...
*/
extern json get_movie_genres(const std::string& language);
extern json get_tv_show_genres(const std::string& language);