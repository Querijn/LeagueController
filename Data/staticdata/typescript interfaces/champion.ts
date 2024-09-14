  interface iChampion {
    id: number;
    name: string;
    key: string;
    roles: ('top' | 'jungle' | 'mid' | 'bot' | 'support')[];
    forms: ChampionForm[];
    transform: 'manual' | 'condition' | 'skill';
  }
  
  interface ChampionForm {
    transform: any; // this will contain transform conditions that automate the change in case of iChampion.transform == 'condition' || 'skill'
    name: string;
    icon: string;
    skills: ChampionSkill[];
    stats: ChampionStats;
  }
  
  //stats
  interface ChampionStats {
    health: ChampionStat;
    healthRegen: ChampionStat;
    mana: ChampionStat;
    manaRegen: ChampionStat;
    armor: ChampionStat;
    magicResistance: ChampionStat;
    attackDamage: ChampionStat;
    attackRange: ChampionStat;
    attackSpeed: ChampionStat;
    attackSpeedBonus: ChampionStat;
    movementSpeed: ChampionStat;
  }
  
  interface ChampionStat {
    flat: number;
    percent: number;
    perLevel: number;
    percentPerLevel: number;
  }
  
  //skill
  interface ChampionSkill {
    key: "A" | "P" | "Q" | "W" | "E" | "R";
    components: SkillComponent[];
    level: SkillLevel;
    transform: 'automatic' | 'manual' | 'none';
  }
  
  interface SkillComponent {
    name: string;
    conditions: any[];
    description: string;
    scaling: ScalingNumber[];
    cost: ScalingNumber;
    range: ScalingNumber;
    cooldown: ScalingNumber;
  }
  
  interface SkillLevel {
    start: number;
    max: number;
    requirement: number[];
  }
  
  //math
  interface ScalingNumber {
    label: string;
    formula: string;
    components: ScalingComponent[];
  }
  
  interface ScalingComponent {
    label: string;
    value: number[];
    scaling: string[];
  }