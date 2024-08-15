import configparser
import math
import subprocess

config = configparser.ConfigParser()
config.sections()
config.read('./tools/AIM.conf')

print("Your config is as follows:")
print("--------")
print({section: dict(config[section]) for section in config.sections()})

INPUT_BITS = int(config["Config"]["Input_Bitwidth"])
ELEMENTS = math.ceil(INPUT_BITS / 31.0)
ALIGN8 = math.ceil(ELEMENTS / 8.0) * 8
VALID_CONF = []
AIE_TOTAL = int(config["Constraints"]["AIE_COLs"]) * int(config["Constraints"]["AIE_ROWs"])
max_perf = 0
max_perf_idx = 0
conf_idx = 0
print("--------")


for r in range(1, int(config["Constraints"]["MAX_ROW"]) + 1):
    if r > int(config["Constraints"]["MAX_ROW"]):
        break
    c = r + 1
    aie_per_pe = r * c
    hw = math.ceil(ALIGN8 / r / 8.0) * 8
    if hw > 1024:
        continue
    AIE_OUT = r + c - 1
    AIE_OUT_CHUNK = math.floor(hw * 31 / 512 + 2)
    TAILS_B = math.ceil((c * hw - math.floor(INPUT_BITS/31))/4) * 4
    AIE_cycle = (hw / 8)**2*9 + 5*hw
    PL_Reorg = math.floor(hw/4 + INPUT_BITS/512*5 + TAILS_B/4) + int(config["Profiling"]["PL_Reorganize"])
    PL_Receive = hw /8 *6 + int(config["Profiling"]["PL_Receive"])
    Merge2 = AIE_OUT*AIE_OUT_CHUNK*2 + int(config["Profiling"]["Merge_Phase2"])
    PL_cycle = max(PL_Reorg, PL_Receive, Merge2)
    PL_Time = PL_cycle*1000/int(config["Config"]["PL_Freq"])
    Task_time = max(PL_Time, AIE_cycle)
    Perf1 = (INPUT_BITS / 32) ** 2 * 2 / Task_time

    In_PLIO = c + r
    Out_PLIO = AIE_OUT
    PLIO_limit = min(math.floor(int(config["Constraints"]["PLIO_IN"]) / In_PLIO), math.floor(int(config["Constraints"]["PLIO_OUT"])))
    AIE_limit = math.floor(AIE_TOTAL/aie_per_pe)

    split_a = r - 1
    split_b = c - 1
    send_plio = r + c
    receive_plio = AIE_OUT
    serialize_merge = receive_plio - 1
    Used_LUTs = int(config["Profiling"]["Non_Repeat_LUTs"]) + split_a * int(config["Profiling"]["Split_input_a"]) \
                + split_b * int(config["Profiling"]["Split_input_b"]) \
                + send_plio * int(config["Profiling"]["Send_PLIO"]) \
                + receive_plio * int(config["Profiling"]["Receive_PLIO"]) \
                + serialize_merge * int(config["Profiling"]["Serialize_Merge"])
                
    PE_limit_LUT = math.floor(int(config["Constraints"]["LUTs"]) / Used_LUTs)

    pe = min(PE_limit_LUT, AIE_limit, PLIO_limit)
    perf_cycle = pe * Perf1
    perf_ddr = int(config["Profiling"]["DDR_Bandwidth"]) * 8 / (INPUT_BITS*4)*(INPUT_BITS/32)**2 * 2
    perf = min(perf_cycle, perf_ddr)

    VALID_CONF.append([INPUT_BITS, pe, hw, hw, 512, 128, int(config["Config"]["PL_Freq"]), perf])

    if max_perf < perf:
        max_perf_idx = conf_idx
        max_perf = perf
    conf_idx += 1


# print(VALID_CONF[max_perf_idx])
optimal_conf = VALID_CONF[max_perf_idx]
print("The optimized configuration has been found!")
print("Generating customized AIM architecture...")
cmd = "python3 tools/gen_array.py " + str(optimal_conf[0]) + " " + str(optimal_conf[1]) + " " + str(optimal_conf[2]) + " " + str(optimal_conf[3]) + " " + str(optimal_conf[4])  + " " + str(optimal_conf[5])  + " " + str(optimal_conf[6])
subprocess.call(cmd, shell=True)

print("Your customized AIM architecture has been generated!")
print("The expected performance is %.2f GOPS" % optimal_conf[7])
