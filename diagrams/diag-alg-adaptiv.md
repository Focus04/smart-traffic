flowchart TD
    %% Noduri Întreruperi (Asincrone)
    subgraph IR_Interrupts ["Tratare Intreruperi Hardware (EXTI)"]
        PB5_14["Senzor IR Detectat (Falling Edge)"]
        Debounce{"Timp Debounce mai mare de 250ms?"}
        IncCounter["Incrementeaza Contor Axa (EW sau NS)"]
        ClearFlag["Curata Flag Intrerupere"]

        PB5_14 --> Debounce
        Debounce -- Da --> IncCounter
        Debounce -- Nu --> ClearFlag
        IncCounter --> ClearFlag
    end

    %% Bucla Principală (Sincronă)
    subgraph Main_Loop ["Bucla Principala (main.c)"]
        FSM_Run["Rulare FSM Semafor"]
        DisplayUpdate["Actualizare LCD/UART (Timp ramas, Contori)"]
        CycleEndCheck{"Final ciclu NS?"}

        subgraph Adaption_Logic ["Algoritm Adaptare (Update_Traffic_Lights)"]
            Compare{"Compara diferenta EW_cnt vs NS_cnt"}
            History{"Verifica istoric (2 cicluri consecutive?)"}
            SetTier["Selecteaza TIER nou (EW_75, NS_90, BASE)"]
            ApplyTimes["Aplica noi durate de Verde"]
            ResetCnt["Reseteaza Contori Trafic"]
        end

        FSM_Run --> DisplayUpdate
        DisplayUpdate --> CycleEndCheck
        CycleEndCheck -- Nu --> FSM_Run
        CycleEndCheck -- Da --> Adaption_Logic

        Compare --> History
        History --> SetTier
        SetTier --> ApplyTimes
        ApplyTimes --> ResetCnt
        ResetCnt --> FSM_Run
    end

    %% Stiluri
    classDef interrupt fill:#f96,stroke:#333,stroke-width:2px,color:white;
    class PB5_14,Debounce,IncCounter,ClearFlag interrupt;