typedef struct phase_step {
  unsigned char power;
  bool reverse;
  double sin;
} phase_step;

typedef struct step_state {
  phase_step a, b;
} step_state;

typedef step_state step_sequence[];