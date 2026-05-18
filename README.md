# Plan de dezvoltare — Hackathon Google Romania (24h, echipă de 3)

> Aplicație fitness Android + Pixel Watch + embedded sensor NuttX
> Teme Google: **Health · Safety · Accessibility**

---

## 1. Filozofia planului

24 de ore = brutal de puțin. Regula de aur: **MVP demonstrabil la ora 20, polish + demo la 20-24**. Nu integrăm "perfect" — integrăm "vizibil". Juriul vede demo-ul, nu codul.

Mapare la temele Google:
- **Health** → planul de antrenament + monitorizare puls/SpO2/calorii
- **Safety** → alertă puls anormal în timpul antrenamentului (HR în afara zonei sigure → notificare + vibrație pe ceas)
- **Accessibility** → TalkBack labels, font scaling, AI coach cu voce (TTS), high-contrast theme

Bifăm toate trei cu efort minim dacă le punem în arhitectură de la început.

---

## 2. Arhitectura aplicației (MVVM + Jetpack)

```
app/
├── data/
│   ├── local/        Room (SQLite): User, Goal, WorkoutSession, HealthSample
│   ├── remote/       Retrofit: GeminiApi, YouTubeApi
│   ├── sensors/      WearDataClient (Pixel Watch), NuttxBleClient
│   └── repository/   UserRepo, WorkoutRepo, FeedRepo, CoachRepo
├── domain/
│   └── usecase/      GeneratePlanUseCase, ComputeCaloriesUseCase, DetectAbnormalHRUseCase
├── ui/
│   ├── onboarding/   OnboardingViewModel + Compose screens
│   ├── home/         HomeViewModel (dashboard + safety alerts)
│   ├── workout/      WorkoutViewModel (live HR/SpO2/steps)
│   ├── coach/        CoachViewModel (Gemini chat)
│   └── feed/         FeedViewModel (YouTube Shorts)
└── di/               Hilt modules

wear/                 Wear OS app (Pixel Watch)
└── HealthServicesClient + DataClient sync
```

**Stack tehnic:**
- Kotlin + Jetpack Compose (Material 3)
- Hilt (DI), Room (persistență), Retrofit + GSON (REST)
- Coroutines + Flow / StateFlow (multithreading + state)
- Health Services API (Wear OS)
- Wear Data Layer API (sync watch ↔ phone)
- Gemini SDK (`com.google.ai.client.generativeai`)
- YouTube Data API v3
- BLE (`android.bluetooth.le`) pentru NuttX

---

## 3. Împărțirea echipei (3 membri)

| Rol | Responsabilități | Stack focus |
|---|---|---|
| **Dev A — Android UI/UX + Arhitectură** | Setup proiect, Navigation, onboarding, dashboard, theming, MVVM scaffold, demo polish | Compose, Navigation, Hilt, ViewModels |
| **Dev B — Wearable & Sensori** | Pixel Watch app (Wear OS), Health Services (HR/SpO2/steps), Data Layer sync, NuttX BLE client, safety alerts | Wear OS, Health Services, BLE |
| **Dev C — AI + Integrări externe** | Gemini coach (chat + plan generation), YouTube Data API (Shorts feed), Room schema, calorie logic | Retrofit, Gemini SDK, YouTube API, Room |

**Toți trei**: la ora 18–20 facem merge + integration testing + demo script împreună.

---

## 4. Timeline 24h

### Faza 0 — Kickoff (0h–2h)
- Toți: aliniere pe scope, repo GitHub, branch protection off (hackathon mode), Android Studio project cu Hilt + Compose template
- **Dev A**: scaffold MVVM + Navigation + theme (Material 3) + 5 placeholder screens
- **Dev B**: creează modul `:wear` (Wear OS app), cere permisiuni Health Services
- **Dev C**: obține API keys (Gemini, YouTube), Retrofit + GSON setup, Room entities draft

**Deliverable la 2h**: app care pornește, navighează între 5 ecrane goale, watch app care emite puls fake.

### Faza 1 — Vertical slice (2h–8h)
- **Dev A**: Onboarding complet (interese sport, vârstă, greutate, frecvență dorită) → salvat în Room. Home dashboard cu carduri statice.
- **Dev B**: Health Services pe ceas → emit HR + steps real → trimite la telefon prin `DataClient.putDataItem`. Telefon ascultă și updatează `HealthRepository` (StateFlow).
- **Dev C**: `GeneratePlanUseCase` apelează Gemini cu profilul user → răspuns JSON cu plan săptămânal → salvat în Room. YouTube search query "fitness shorts" → afișat ca listă.

**Deliverable la 8h**: end-to-end vizibil pe câte un flow per dev.

### Faza 2 — Integrare + features cheie (8h–14h)
- **Dev A**: ecran Workout live (afișează HR/SpO2/steps din StateFlow), ecran Coach chat UI, ecran Feed (LazyColumn cu thumbnails + WebView/ExoPlayer pentru Shorts)
- **Dev B**: NuttX BLE — scan + connect + caracteristică custom de citire (dacă NuttX device-ul are firmware gata; dacă nu, mock layer cu același interface). Safety: detectează HR > 180 sau < 40 → emite event → notificare local.
- **Dev C**: Gemini chat streaming în CoachViewModel (system prompt cu profilul user-ului ca context). Calorii: formulă MET-based pe baza HR + greutate + durată.

**Deliverable la 14h**: toate ecranele funcționale cu date reale.

### Faza 3 — Accessibility + Safety polish (14h–18h)
- **Dev A**: `contentDescription` peste tot, TalkBack test, font scaling, dark theme, dynamic color
- **Dev B**: notificare safety + vibrație pe ceas când HR anormal. Test cu device real.
- **Dev C**: TTS pentru răspunsurile coach-ului (accessibility win), prompt engineering Gemini să dea răspunsuri scurte și acționabile

**Deliverable la 18h**: feature complete, fără bug-uri majore.

### Faza 4 — Demo prep (18h–22h)
- Toți: bug bash, demo script (ce arătăm și în ce ordine), seed data realistă (nu user gol), backup video în caz de fail live
- Demo flow: splash → onboarding showcase → home → workout live cu watch pe mâna unuia → safety alert simulat → coach AI răspunde → feed shorts
- Pregătim o "demo mode" toggle care injectează date credibile dacă watch-ul/NuttX cedează

### Faza 5 — Buffer + prezentare (22h–24h)
- 1h buffer absolut pentru disasters
- Slides / pitch: problema → temele Google bifate → demo → tech stack → ce ar urma

---

## 5. Scope cuts (de la nice-to-have la cut-first)

Dacă rămânem în urmă, tăiem în această ordine:
1. **NuttX device** (cel mai riscant — mock-uim dacă nu merge la 12h)
2. **YouTube Shorts player real** → înlocuim cu thumbnails + open in YouTube app
3. **Gemini streaming** → simple request/response
4. **Wear OS app standalone** → doar Health Services pe telefon (tăiem complet ceasul dacă pairing-ul nu merge)

---

## 6. Riscuri concrete & mitigări

| Risc | Mitigare |
|---|---|
| Pixel Watch pairing eșuează | Health Services rulează și pe telefon → date din senzorii telefonului |
| NuttX firmware nu e gata | Mock `NuttxBleClient` cu același interface, comutare prin DI |
| Gemini quota / network | Cache răspunsuri, fallback la planuri hardcoded |
| YouTube API quota (10k/day default) | Cache rezultate în Room, search-uri puține |
| Demo crash live | Filmăm backup video la 21h |

---

## 7. Convenții de echipă

- **Branching**: `main` protejat doar verbal; fiecare lucrează pe `dev/<nume>` și face PR fără review formal — merge rapid după smoke test local
- **Commits**: prefix `[A]`, `[B]`, `[C]` ca să știm rapid cine ce-a atins
- **Sync**: standup de 5 minute la fiecare 4h (00h, 04h, 08h, 12h, 16h, 20h)
- **Slack/Discord**: canal dedicat, partajăm screenshot-uri din emulator după fiecare feature
- **Secrets**: API keys în `local.properties` (nu commit-uim), `.gitignore` setat din primul commit
- **Demo device**: alegem din primul moment ce telefon + ceas folosim la demo, le ținem încărcate

---

## 8. Checklist de start (primele 2h)

- [ ] Repo GitHub creat, toți 3 cu acces
- [ ] Android Studio project nou: Empty Compose Activity, min SDK 26, Kotlin
- [ ] Adăugat Hilt, Room, Retrofit, Coroutines, Navigation Compose în `build.gradle`
- [ ] Modul `:wear` adăugat
- [ ] API key Gemini obținut și pus în `local.properties`
- [ ] API key YouTube Data v3 obținut
- [ ] Schema Room: `User`, `Goal`, `WorkoutSession`, `HealthSample` (entități goale)
- [ ] 5 screens placeholder: Onboarding, Home, Workout, Coach, Feed
- [ ] Watch app care logăea HR fake la fiecare 2s
- [ ] `.gitignore` cu `local.properties`, `*.keystore`, `/build`