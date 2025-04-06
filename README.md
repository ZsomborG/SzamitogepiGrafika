
Féléves beadandó leírása. Alapvetően C nyelven irt, MVC struktúrára alapozva, OpenGL és SDL2 alapokkal kiindulva számítógépi grafika kurzusra.

---

## Projekt leírás

Alapkoncepció: "autósakk" féle játékok (TeamFight Tactics, Dota AutoChess, Hearthstone Battleground)

- **Cél**: Túlélni az AI ellenség hullámait az egységek stratégiai megvásárlásával, elhelyezésével és fejlesztésével egy rácsos táblán.

### Játékmenet

A játékmenet körforgása:

1. **Előkészítő fázis**: Minimális arannyal kezdődik a játék. Egy boltban egy kis választékot kapsz bizonyos egységekből. Megveszed az egységeket, és elhelyezed őket a négyzetrácsos tábla saját oldalán. A már a táblán lévő egységeket mozgathatod.
2. **Harci fázis**: A harc automatikusan (vagy egy gomb segítségével) kezdődik. A te egységeid és az előre meghatározott AI-hullám egységei automatikusan harcolnak egyszerű szabályok alapján (támadd meg a legközelebbi ellenséget a hatótávolságon belül). Az egységek alapvető statisztikákkal rendelkeznek (élet, sebzés, támadási sebesség, hatótávolság). Az utolsó állva maradt egységekkel rendelkező csapat nyeri a kört.
3. **Harc utáni küzdelem**: Ha veszítesz, akkor a túlélő ellenséges egységek sebzése alapján kapsz kárt. Ha nyersz akkor fix aranyat kapsz a következő fordulóra. Az AI hullám erősödik. Ismétlődik, amíg a játékos HP-je 0 nem lesz.

### Alapvető funkciók

- Négyzetrácsos táblán helyezel el egységeket.
- Néhány különböző egységtípussal (pl. közelharci tank, távolsági íjász).
- Egyszerűsített statisztikák: élet, támadási ráta, hatótávolság. Esetleg páncélzat (egyszerű sebzéscsökkentés).
- Fix arany körönként. Fix egységköltségek.
- Fix számú egységhely a táblán.
- Egyszerű bolt: Fix számú egységhely (pl. 3 felkínált egység).
- Csak játékos kontra AI hullámok: Nincs PvP-szimuláció. Előre meghatározott hullámok minden körre.
- Egyszerűsített harci mesterséges intelligencia: Az egységek a legközelebbi ellenséges egységet veszik célba. Mozognak, majd támadnak.

### Feltételek

- **Kamerakezelés**: Enyhén ferde statikus nézet. Zoomolás (egérkerék) a tábla felett, bizonyos zoom után a kamera szabad mozgatása WASD-vel.
- **Objektumok**: Az egységek betöltött .obj modellek. Maga a tábla egy egyszerű textúrázott cube.
- **Animáció, mozgatás**:
  - **Interakció**: Kattintás a bolt UI-ra a vásárláshoz, Húzás/kattintás az egységek elhelyezéséhez a táblarácson. Kattintás a „Start Combat” gombra.
  - **Animáció**: Harc közben az egységek enyhén mozoghatnak a célpontjuk felé, egyszerű támadási animációt végezhetnek (pl. gyors skála fel/le, színvillanás), és halálukkor eltűnhetnek. Az egységek feletti HP-sávok csökkennek.
- **Textúrák**: Egyedi textúrák minden egységtípushoz, textúra(k) a táblához. UI textúrák az ImGui segítségével.
- **Fények**: Szabványos jelenetvilágítás, +/- segítségével állítható.
- **Használati útmutató (F1)**: Az egyszerűsített szabályok, egységtípusok/statisztikák, fázisok és vezérlők magyarázata egy ImGui ablakon keresztül.

### Többlet funkciók jobb jegyhez

- **Bonyolultabb animációk**: Különböző támadási animációk hozzáadása a puszta villogás helyett. (Egyszerű egységképességek megvalósítása vizuális effektekkel.)
- **Részecskerendszerek**: Részecskeeffekt hozzáadása a támadásokhoz, egységhalálhoz vagy egyszerű képességekhez.
- **AI**: AI harci célzásának a javítása (pl. célozzanak bizonyos egységek a legalacsonyabb HP-ra vagy a legtávolabbi egységre).
- **Köd**: Minden kör után megjelenik pár másodpercing a tábla négy oldalán.
- **Átlátszóság**: Használható vizuális effektekhez vagy az egységek támadási sugarának jelzéséhez.
- **Árnyék**: A táblán lévő egységeknek egyszerű árnyékok.
- **Ütközésvizsgálat**: Implicit módon már most is szükséges a tábla lapkáira vagy egységeire történő egérkattintáshoz. Az egységek körüli határoló dobozok hasznosak (bounding boxes).
- **Stencil puffer**: Használható a kiválasztott egységek kiemelésére vagy érvényes elhelyezési rácsokhoz.
- **Felhasználói felület**: Minimálisan eleve integrálva van az ImGui-ba a bolt, játékos statisztikák, stb.
- **Objektumok kijelölése egérrel**: Lényeges az egységek elhelyezéséhez.
- **Térkép kezelése**: A „térkép” a játéktábla rácshálója. Nem heightmap alapú.
- **Procedurális geometria, textúrák**: Procedurálisan generált játéktábla mintáit.

### Egyéb fejlesztések a játék funkcionalitásának kibővítéséhez

- **Képességek**: Egységek támadási lehetőségeit kibőviteni képességekkel, manával és ezzel együtt az AI-t.
- **Változatosság**: Több egység "tier" bevezetése (különböző erősségű képességek, amiket az egység "tier"-je jelez az UI-on: pl. bronz, ezüst, arany)
- **Tábla bővítése**: Négyzetrácsos elhelyezés helyett hexagonális vagy más sokszögű forma ismétlődése.
- **Item**: Item-ek bevezése, ezeket az egységre lehet "adni", hogy erősebbek legyenek.
- **Synergies/Traits**: 2-3 ugyan abból a fajból/családból származó egység bónuszt ad egymásnak (plusz élet vagy támadás).
- **Arany**: A körök végén kapott vagy a játékosnál lévő arany valamilyen szintű kamatozása.
- **Játékos szintek bevezetése**: Aranyért cserébe a játékos tud venni egy szintet, ezzel még egy egységet le tud rakni a pályára. Új egységek vásárlási lehetőségének a feloldása.
- **Egység szintek bevezése**: 3 darab ugyan olyan szintű és fajta egység erősebb lesz (3db 1 csillagos A egységből lesz 1db 2 csillagos A egység, ami picivel erősebb, max 3 csillagos)
- **Bolt**: Új bolti felhozatalt lehet kérni aranyért cserébe (fixed számú elérhető egységek a játék során, hogy nehezebb legyen 3 csillagos egységeket elérni)
- **"Pad" (bench)**: A megvásárolt egységeket nem kötelező egyből a pályára tenni, hanem a padra lehet ültetni őket, majd később eladni vagy lerakni.

| Funkció                    | (Tervezett) Használt könyvtár            |
| :-------------------------- | :------------------------------------------ |
| **Core/Window/Input** | SDL2                                        |
| **Graphics API**      | OpenGL                                      |
| **GL Loading**        | GLAD                                        |
| **Math**              | cglm                                        |
| **UI**                | Dear ImGui                                  |
| **Model Loading**     | Assimp                                      |
| **Texture Loading**   | stb\_image.h                                |
| **AI Pathfinding**    | A\* (Implement)                             |
| **AI Logic**          | State Machine/Behavior Tree (Implement/Lib) |
